#include "uci.h"
#include "search.h"
#include "movegen.h"
#include "board.h"
#include "book.h"
#include "constants.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <limits>
#include <cctype>

// Global board state and RNG
BoardState currentBoard;
std::mt19937 global_rng;

// Apply move with full game logic (halfmove clock, position history, etc.)
void master_apply_move(const Move& move) {
    char piece = currentBoard.board[move.fromRow][move.fromCol];
    char captured = currentBoard.board[move.toRow][move.toCol];
    bool isPawn = (toupper(piece) == W_PAWN);
    bool isCap = (captured != EMPTY) || move.isEnPassantCapture;
    apply_raw_move_to_board(currentBoard, move);
    if (isPawn || isCap) { currentBoard.halfmoveClock = 0; } else { currentBoard.halfmoveClock++; }
    if (!currentBoard.whiteToMove) { currentBoard.fullmoveNumber++; }
    currentBoard.addCurrentPositionToHistory();
}

// Game end checks
bool isCheckmate() { std::vector<Move> m; generateLegalMoves(currentBoard, m, false); return m.empty() && isKingInCheck(currentBoard, currentBoard.whiteToMove); }
bool isStalemate() { std::vector<Move> m; generateLegalMoves(currentBoard, m, false); return m.empty() && !isKingInCheck(currentBoard, currentBoard.whiteToMove); }
bool isThreefoldRepetition() { return currentBoard.positionCounts[currentBoard.currentFenKey] >= 3; }
bool isFiftyMoveDraw() { return currentBoard.halfmoveClock >= 100; }

std::string checkGameEndStatus() {
    if (isCheckmate()) return currentBoard.whiteToMove ? "0-1 {Black mates}" : "1-0 {White mates}";
    if (isStalemate()) return "1/2-1/2 {Stalemate}";
    if (isThreefoldRepetition()) return "1/2-1/2 {Draw by threefold repetition}";
    if (isFiftyMoveDraw()) return "1/2-1/2 {Draw by fifty-move rule}";
    return "";
}

// UCI handlers
void handleUci() {
    // Load opening book if not already loaded
    if (globalBook.size() == 0) {
        globalBook.loadFromFile("opening_book.txt");
    }
    std::cout << "id name Gotham\nid author Outhills\nuciok" << std::endl;
}
void handleIsReady() { std::cout << "readyok" << std::endl; }
void handleUciNewGame() {
    currentBoard.reset();
    transpositionTable.clear();
}

void handlePosition(std::istringstream& iss) {
    std::string token, fen_str; iss >> token;
    if (token == "startpos") {
        currentBoard.reset();
        transpositionTable.clear();
        iss >> token;
    } else if (token == "fen") {
        while(iss >> token && token != "moves") { fen_str += token + " "; }
        if (!fen_str.empty()) fen_str.pop_back();
        currentBoard.parseFen(fen_str);
        transpositionTable.clear();
    }
    if (token == "moves") {
        while (iss >> token) {
            Move pMove; if (token.length() < 4) continue;
            pMove.fromCol = token[0] - 'a'; pMove.fromRow = '8' - token[1];
            pMove.toCol = token[2] - 'a'; pMove.toRow = '8' - token[3];
            pMove.promotionPiece = EMPTY;
            if (token.length() == 5) {
                char promo = token[4]; char pieceColor = currentBoard.whiteToMove ? 'W' : 'B';
                if (promo == 'q') pMove.promotionPiece = (pieceColor == 'W' ? W_QUEEN : B_QUEEN);
                else if (promo == 'r') pMove.promotionPiece = (pieceColor == 'W' ? W_ROOK : B_ROOK);
                else if (promo == 'b') pMove.promotionPiece = (pieceColor == 'W' ? W_BISHOP : B_BISHOP);
                else if (promo == 'n') pMove.promotionPiece = (pieceColor == 'W' ? W_KNIGHT : B_KNIGHT);
            }
            std::vector<Move> legal_moves; generateLegalMoves(currentBoard, legal_moves, false);
            Move moveToApply; bool found = false;
            for (const auto& legal_m : legal_moves) {
                if (legal_m.fromRow == pMove.fromRow && legal_m.fromCol == pMove.fromCol &&
                    legal_m.toRow == pMove.toRow && legal_m.toCol == pMove.toCol &&
                    legal_m.promotionPiece == pMove.promotionPiece ) {
                    moveToApply = legal_m; found = true; break;
                }
            }
            if (found) { master_apply_move(moveToApply); } else { break; }
        }
    }
}

void handleGo(std::istringstream& iss) {
    std::string token;
    long long wtime_ms = -1, btime_ms = -1, winc_ms = 0, binc_ms = 0;
    int movestogo = 0;
    long long movetime_ms = -1;

    while(iss >> token) {
        if (token == "wtime") iss >> wtime_ms;
        else if (token == "btime") iss >> btime_ms;
        else if (token == "winc") iss >> winc_ms;
        else if (token == "binc") iss >> binc_ms;
        else if (token == "movestogo") iss >> movestogo;
        else if (token == "movetime") iss >> movetime_ms;
    }

    long long allocated_ms;
    long long time_buffer_ms = 100;

    if (movetime_ms != -1) {
        allocated_ms = std::max(10LL, movetime_ms - time_buffer_ms);
    } else {
        long long my_time = currentBoard.whiteToMove ? wtime_ms : btime_ms;
        long long my_inc = currentBoard.whiteToMove ? winc_ms : binc_ms;
        if (my_time != -1) {
             int moves_remaining = (movestogo > 0 && movestogo < 80) ? movestogo : 35;
             allocated_ms = (my_time / moves_remaining) + my_inc - time_buffer_ms;
             allocated_ms = std::min(allocated_ms, my_time / 2 - time_buffer_ms);
             allocated_ms = std::max(10LL, allocated_ms);
        } else {
            allocated_ms = 2000 - time_buffer_ms;
        }
    }

    std::chrono::milliseconds timeLimit(allocated_ms);

    auto startTime = std::chrono::steady_clock::now();
    time_is_up.store(false, std::memory_order_relaxed);
    nodes_searched.store(0, std::memory_order_relaxed);

    // Clear killer moves and history table for new search
    clearKillerMoves();
    clearHistoryTable();

    std::vector<Move> legalEngineMoves;
    generateLegalMoves(currentBoard, legalEngineMoves, false);
    if (legalEngineMoves.empty()) { std::cout << "bestmove 0000" << std::endl; return; }

    // Check opening book first
    Move bookMove;
    if (globalBook.probeBook(currentBoard, bookMove)) {
        std::cout << "info string Book move" << std::endl;
        std::cout << "bestmove " << bookMove.toUci() << std::endl;
        return;
    }

    orderMoves(currentBoard, legalEngineMoves, 0);

    Move bestMoveOverall = legalEngineMoves[0];
    Move bestMoveThisIteration = legalEngineMoves[0];
    int bestEvalOverall = std::numeric_limits<int>::min();

    bool isEngineWhite = currentBoard.whiteToMove;

    // Iterative Deepening Loop with Aspiration Windows
    for (int currentDepth = 1; currentDepth <= MAX_SEARCH_PLY; ++currentDepth) {
        auto iterationStartTime = std::chrono::steady_clock::now();
        int currentIterBestEval = std::numeric_limits<int>::min();
        std::vector<Move> candidateBestMovesThisIteration;

        uint64_t nodes_at_start_of_iter = nodes_searched.load(std::memory_order_relaxed);

        // Aspiration windows for depths >= 3
        int alpha = std::numeric_limits<int>::min();
        int beta = std::numeric_limits<int>::max();
        if (currentDepth >= ASPIRATION_MIN_DEPTH && bestEvalOverall != std::numeric_limits<int>::min()) {
            alpha = bestEvalOverall - ASPIRATION_WINDOW;
            beta = bestEvalOverall + ASPIRATION_WINDOW;
        }

        for (const auto& engineMove : legalEngineMoves) {
            BoardState boardAfterEngineMove = currentBoard;
            apply_raw_move_to_board(boardAfterEngineMove, engineMove);
            int evalFromWhitePerspective = alphaBetaSearch(boardAfterEngineMove, currentDepth - 1,
                                                           alpha, beta,
                                                           !isEngineWhite,
                                                           startTime, timeLimit, 1, true);

            if (time_is_up.load(std::memory_order_relaxed)) break;

            // Re-search with full window if we fall outside aspiration window
            if ((evalFromWhitePerspective <= alpha || evalFromWhitePerspective >= beta) && currentDepth >= ASPIRATION_MIN_DEPTH) {
                evalFromWhitePerspective = alphaBetaSearch(boardAfterEngineMove, currentDepth - 1,
                                                           std::numeric_limits<int>::min(),
                                                           std::numeric_limits<int>::max(),
                                                           !isEngineWhite,
                                                           startTime, timeLimit, 1, true);
            }

            if (time_is_up.load(std::memory_order_relaxed)) break;

            int currentMoveScoreForEngine;
            if (isEngineWhite) {
                currentMoveScoreForEngine = evalFromWhitePerspective;
            } else {
                currentMoveScoreForEngine = -evalFromWhitePerspective;
            }

            if (currentMoveScoreForEngine > currentIterBestEval) {
                currentIterBestEval = currentMoveScoreForEngine;
                candidateBestMovesThisIteration.clear();
                candidateBestMovesThisIteration.push_back(engineMove);
            } else if (currentMoveScoreForEngine == currentIterBestEval) {
                candidateBestMovesThisIteration.push_back(engineMove);
            }
        }

        if (time_is_up.load(std::memory_order_relaxed)) { break; }

        if (!candidateBestMovesThisIteration.empty()) {
            std::uniform_int_distribution<int> distrib(0, candidateBestMovesThisIteration.size() - 1);
            bestMoveThisIteration = candidateBestMovesThisIteration[distrib(global_rng)];
            bestMoveOverall = bestMoveThisIteration;
            bestEvalOverall = currentIterBestEval;

            auto iterationEndTime = std::chrono::steady_clock::now();
            auto iterationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(iterationEndTime - iterationStartTime);
            uint64_t nodes_this_iter = nodes_searched.load(std::memory_order_relaxed) - nodes_at_start_of_iter;
            uint64_t nps = (iterationDuration.count() > 0) ? (nodes_this_iter * 1000 / iterationDuration.count()) : 0;

            int uci_score_val = bestEvalOverall;
            std::string uci_score_type = "cp";

            // Refined mate score reporting
            if (abs(uci_score_val) > MATE_SCORE - MAX_SEARCH_PLY * 2 ) {
                uci_score_type = "mate";
                int ply_to_mate_from_root = MATE_SCORE - abs(uci_score_val);
                int moves_to_mate = (ply_to_mate_from_root + currentDepth + 1) / 2;
                uci_score_val = (bestEvalOverall > 0) ? moves_to_mate : -moves_to_mate;
            }

            std::cout << "info depth " << currentDepth
                      << " score " << uci_score_type << " " << uci_score_val
                      << " time " << iterationDuration.count()
                      << " nodes " << nodes_this_iter
                      << " nps " << nps
                      << " pv " << bestMoveOverall.toUci() << std::endl;

        } else { break; }

        if (std::chrono::steady_clock::now() - startTime >= timeLimit) { break; }
        if (abs(bestEvalOverall) >= MATE_SCORE - MAX_SEARCH_PLY*2) { break; }

    } // End Iterative Deepening Loop

    std::cout << "bestmove " << bestMoveOverall.toUci() << std::endl;
}
