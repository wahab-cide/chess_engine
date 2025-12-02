#include "search.h"
#include "evaluation.h"
#include "movegen.h"
#include "board.h"
#include "constants.h"
#include <limits>
#include <algorithm>

// Global search state
std::atomic<bool> time_is_up{false};
std::atomic<uint64_t> nodes_searched{0};
std::map<std::string, TTEntry> transpositionTable;

// Quiescence search
int quiescenceSearch(BoardState state, int alpha, int beta, bool maximizingPlayer,
                     const std::chrono::steady_clock::time_point& startTime,
                     const std::chrono::milliseconds& timeLimit, int quiescenceDepth) {
    if (time_is_up.load(std::memory_order_relaxed)) return 0;
    nodes_searched++;

    const uint64_t CHECK_TIME_MASK = 1023;
    if ((nodes_searched.load(std::memory_order_relaxed) & CHECK_TIME_MASK) == 0) {
        if (std::chrono::steady_clock::now() - startTime >= timeLimit) {
            time_is_up.store(true, std::memory_order_relaxed);
            return 0;
        }
    }
    if (quiescenceDepth <= 0) return evaluateBoard(state);

    int stand_pat = evaluateBoard(state);
    bool in_check = isKingInCheck(state, state.whiteToMove);

    if (in_check) {
        if (maximizingPlayer) stand_pat -= IN_CHECK_PENALTY;
        else stand_pat += IN_CHECK_PENALTY;
    }

    if (maximizingPlayer) {
        if (stand_pat >= beta && !in_check) return beta;
        alpha = std::max(alpha, stand_pat);
    } else {
        if (stand_pat <= alpha && !in_check) return alpha;
        beta = std::min(beta, stand_pat);
    }

    std::vector<Move> q_moves;
    generateLegalMoves(state, q_moves, !in_check);
    orderMoves(state, q_moves);

    if (in_check && q_moves.empty()) {
        return maximizingPlayer ? (-MATE_SCORE - MAX_SEARCH_PLY - quiescenceDepth) : (MATE_SCORE + MAX_SEARCH_PLY + quiescenceDepth);
    }
    if (!in_check && q_moves.empty()) {
        return stand_pat;
    }

    if (maximizingPlayer) {
        for (const auto& move : q_moves) {
            BoardState nextState = state;
            apply_raw_move_to_board(nextState, move);
            int score = quiescenceSearch(nextState, alpha, beta, false, startTime, timeLimit, quiescenceDepth - 1);
            if (time_is_up.load(std::memory_order_relaxed)) return 0;
            alpha = std::max(alpha, score);
            if (alpha >= beta) break;
        }
        return alpha;
    } else {
        for (const auto& move : q_moves) {
            BoardState nextState = state;
            apply_raw_move_to_board(nextState, move);
            int score = quiescenceSearch(nextState, alpha, beta, true, startTime, timeLimit, quiescenceDepth - 1);
            if (time_is_up.load(std::memory_order_relaxed)) return 0;
            beta = std::min(beta, score);
            if (alpha >= beta) break;
        }
        return beta;
    }
}

// Alpha-beta search
int alphaBetaSearch(BoardState state, int depth, int alpha, int beta, bool maximizingPlayer,
                    const std::chrono::steady_clock::time_point& startTime,
                    const std::chrono::milliseconds& timeLimit)
{
    if (time_is_up.load(std::memory_order_relaxed)) return 0;
    nodes_searched++;

    std::string currentKey = state.currentFenKey;
    auto tt_it = transpositionTable.find(currentKey);
    if (tt_it != transpositionTable.end()) {
        TTEntry& entry = tt_it->second;
        if (entry.depth >= depth) {
            if (entry.flag == TT_EXACT) return entry.score;
            if (entry.flag == TT_LOWERBOUND && entry.score >= beta) return entry.score;
            if (entry.flag == TT_UPPERBOUND && entry.score <= alpha) return entry.score;
        }
    }

    std::vector<Move> legalMoves;
    generateLegalMoves(state, legalMoves, false);

    if (legalMoves.empty()) {
        if (isKingInCheck(state, state.whiteToMove)) return maximizingPlayer ? (-MATE_SCORE - depth) : (MATE_SCORE + depth);
        else return DRAW_SCORE;
    }
    if (state.positionCounts[currentKey] >= 3 || state.halfmoveClock >= 100) return DRAW_SCORE;

    if (depth == 0) {
        return quiescenceSearch(state, alpha, beta, maximizingPlayer, startTime, timeLimit, MAX_QUIESCENCE_PLY);
    }

    const uint64_t CHECK_TIME_MASK = 1023;
    if ((nodes_searched.load(std::memory_order_relaxed) & CHECK_TIME_MASK) == 0) {
        if (std::chrono::steady_clock::now() - startTime >= timeLimit) {
            time_is_up.store(true, std::memory_order_relaxed);
            return 0;
        }
    }

    orderMoves(state, legalMoves);
    TTEntryFlag bestFlag = TT_UPPERBOUND;
    int movesSearchedCount = 0;
    bool inCheck = isKingInCheck(state, state.whiteToMove);

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : legalMoves) {
            BoardState nextState = state;
            apply_raw_move_to_board(nextState, move);

            int currentEval;
            int newDepth = depth - 1;
            bool givesCheck = isKingInCheck(nextState, nextState.whiteToMove);

            // Check Extension
            if (givesCheck && depth < MAX_SEARCH_PLY) {
                newDepth += CHECK_EXTENSION_PLY;
            }

            // Late Move Reduction (LMR)
            bool applyLmr = false;
            if (depth >= LMR_MIN_DEPTH_FOR_REDUCTION &&
                movesSearchedCount >= LMR_MIN_MOVES_TO_TRY_REDUCTION &&
                !move.isCapture(state) &&
                move.promotionPiece == EMPTY &&
                !inCheck &&
                !givesCheck) {
                applyLmr = true;
            }

            if (applyLmr) {
                currentEval = alphaBetaSearch(nextState, newDepth - LMR_REDUCTION, alpha, beta, false, startTime, timeLimit);
            } else {
                currentEval = alphaBetaSearch(nextState, newDepth, alpha, beta, false, startTime, timeLimit);
            }

            if (time_is_up.load(std::memory_order_relaxed)) return 0;

            // Re-search if LMR was applied and the score is promising
            if (applyLmr && currentEval > alpha) {
                 currentEval = alphaBetaSearch(nextState, newDepth, alpha, beta, false, startTime, timeLimit);
                 if (time_is_up.load(std::memory_order_relaxed)) return 0;
            }

            if (currentEval > maxEval) maxEval = currentEval;

            if (currentEval > alpha) {
                alpha = currentEval;
                bestFlag = TT_EXACT;
            }
            if (beta <= alpha) {
                 bestFlag = TT_LOWERBOUND;
                 break;
            }
            movesSearchedCount++;
        }
        if (!time_is_up.load(std::memory_order_relaxed) && (transpositionTable.size() < MAX_TT_SIZE || tt_it != transpositionTable.end())) {
            TTEntry newEntry; newEntry.score = maxEval; newEntry.depth = depth; newEntry.flag = bestFlag;
            transpositionTable[currentKey] = newEntry;
        }
        return maxEval;
    } else { // Minimizing Player
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : legalMoves) {
            BoardState nextState = state; apply_raw_move_to_board(nextState, move);
            int currentEval;
            int newDepth = depth - 1;
            bool givesCheck = isKingInCheck(nextState, nextState.whiteToMove);

            // Check Extension
            if (givesCheck && depth < MAX_SEARCH_PLY) {
                newDepth += CHECK_EXTENSION_PLY;
            }

            // Late Move Reduction (LMR)
            bool applyLmr = false;
            if (depth >= LMR_MIN_DEPTH_FOR_REDUCTION &&
                movesSearchedCount >= LMR_MIN_MOVES_TO_TRY_REDUCTION &&
                !move.isCapture(state) &&
                move.promotionPiece == EMPTY &&
                !inCheck &&
                !givesCheck) {
                applyLmr = true;
            }

            if (applyLmr) {
                 currentEval = alphaBetaSearch(nextState, newDepth - LMR_REDUCTION, alpha, beta, true, startTime, timeLimit);
            } else {
                 currentEval = alphaBetaSearch(nextState, newDepth, alpha, beta, true, startTime, timeLimit);
            }

            if (time_is_up.load(std::memory_order_relaxed)) return 0;

            // Re-search for LMR
            if (applyLmr && currentEval < beta) {
                 currentEval = alphaBetaSearch(nextState, newDepth, alpha, beta, true, startTime, timeLimit);
                 if (time_is_up.load(std::memory_order_relaxed)) return 0;
            }

            if (currentEval < minEval) minEval = currentEval;

            if (currentEval < beta) {
                beta = currentEval;
                bestFlag = TT_EXACT;
            }
            if (beta <= alpha) {
                bestFlag = TT_UPPERBOUND;
                break;
            }
            movesSearchedCount++;
        }
        if (!time_is_up.load(std::memory_order_relaxed) && (transpositionTable.size() < MAX_TT_SIZE || tt_it != transpositionTable.end())) {
            TTEntry newEntry; newEntry.score = minEval; newEntry.depth = depth; newEntry.flag = bestFlag;
            transpositionTable[currentKey] = newEntry;
        }
        return minEval;
    }
}
