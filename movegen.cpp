#include "movegen.h"
#include "board.h"
#include "constants.h"
#include "search.h"
#include <algorithm>
#include <cctype>

// Add a move to the list after validation
void addMove(const BoardState& s, int r1, int c1, int r2, int c2, std::vector<Move>& m,
             char promo, bool ksc, bool qsc, bool ep) {
    if(!isSquareOnBoard(r1,c1) || !isSquareOnBoard(r2,c2)) return;
    char piece=getPieceAt(s,r1,c1), target=getPieceAt(s,r2,c2);
    if(piece==EMPTY || (s.whiteToMove && isWhitePiece(target)) || (!s.whiteToMove && isBlackPiece(target))) return;
    m.emplace_back(r1,c1,r2,c2,promo,ksc,qsc,ep);
}

// Generate pawn moves
void generatePawnMoves(const BoardState& state, int r, int c, std::vector<Move>& moves, bool capturesOnly) {
    char piece = state.board[r][c];
    int direction = (isWhitePiece(piece)) ? -1 : 1;
    char promotionPieces[] = {state.whiteToMove ? W_QUEEN : B_QUEEN, state.whiteToMove ? W_ROOK : B_ROOK,
                              state.whiteToMove ? W_BISHOP : B_BISHOP, state.whiteToMove ? W_KNIGHT : B_KNIGHT};
    int promotion_rank = state.whiteToMove ? 0 : 7;
    int start_rank = state.whiteToMove ? 6 : 1;
    if (!capturesOnly && isSquareOnBoard(r + direction, c) && state.board[r + direction][c] == EMPTY) {
        if (r + direction == promotion_rank) { for (char promo : promotionPieces) addMove(state, r, c, r + direction, c, moves, promo); }
        else { addMove(state, r, c, r + direction, c, moves); }
        if (r == start_rank && isSquareOnBoard(r + 2 * direction, c) && state.board[r + 2 * direction][c] == EMPTY) {
            addMove(state, r, c, r + 2 * direction, c, moves);
        }
    }
    for (int dc : {-1, 1}) {
        int capture_r = r + direction; int capture_c = c + dc;
        if (isSquareOnBoard(capture_r, capture_c)) {
            char targetPiece = state.board[capture_r][capture_c];
            bool canCapture = (state.whiteToMove && isBlackPiece(targetPiece)) || (!state.whiteToMove && isWhitePiece(targetPiece));
            if (canCapture) {
                if (capture_r == promotion_rank) { for (char promo : promotionPieces) addMove(state, r, c, capture_r, capture_c, moves, promo); }
                else { addMove(state, r, c, capture_r, capture_c, moves); }
            }
            if (capture_r == state.enPassantTarget.first && capture_c == state.enPassantTarget.second && targetPiece == EMPTY) {
                 addMove(state, r, c, capture_r, capture_c, moves, EMPTY, false, false, true);
            }
        }
    }
}

// Generate sliding piece moves (rook, bishop, queen)
void generateSlidingMoves(const BoardState& state, int r, int c, std::vector<Move>& moves,
                         const std::vector<std::pair<int, int>>& directions, bool capturesOnly) {
    char piece = state.board[r][c];
    for (auto dir : directions) {
        for (int i = 1; i < 8; ++i) {
            int next_r = r + dir.first * i; int next_c = c + dir.second * i;
            if (!isSquareOnBoard(next_r, next_c)) break;
            char targetPiece = state.board[next_r][next_c];
            if (targetPiece == EMPTY) { if (!capturesOnly) addMove(state, r, c, next_r, next_c, moves); }
            else {
                if ((isWhitePiece(piece) && isBlackPiece(targetPiece)) || (isBlackPiece(piece) && isWhitePiece(targetPiece))) {
                    addMove(state, r, c, next_r, next_c, moves);
                }
                break;
            }
        }
    }
}

// Generate knight moves
void generateKnightMoves(const BoardState& state, int r, int c, std::vector<Move>& moves, bool capturesOnly) {
    const std::vector<std::pair<int, int>> knight_deltas = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (auto d : knight_deltas) {
        if (capturesOnly) { if(isSquareOnBoard(r+d.first, c+d.second) && getPieceAt(state, r+d.first, c+d.second) != EMPTY) addMove(state, r, c, r + d.first, c + d.second, moves); }
        else { addMove(state, r, c, r + d.first, c + d.second, moves); }
    }
}

// Generate king moves
void generateKingMoves(const BoardState& state, int r, int c, std::vector<Move>& moves, bool capturesOnly) {
    const std::vector<std::pair<int, int>> king_deltas = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    for (auto d : king_deltas) {
        if (capturesOnly) { if(isSquareOnBoard(r+d.first, c+d.second) && getPieceAt(state, r+d.first, c+d.second) != EMPTY) addMove(state, r, c, r + d.first, c + d.second, moves); }
        else { addMove(state, r, c, r + d.first, c + d.second, moves); }
    }
    if (!capturesOnly) {
        if (state.whiteToMove) {
            if (state.whiteKingSideCastle && state.board[7][5]==EMPTY && state.board[7][6]==EMPTY &&
                !isSquareAttacked(state, 7, 4, false) && !isSquareAttacked(state, 7, 5, false) && !isSquareAttacked(state, 7, 6, false)) {
                addMove(state, 7, 4, 7, 6, moves, EMPTY, true, false, false);
            }
            if (state.whiteQueenSideCastle && state.board[7][1]==EMPTY && state.board[7][2]==EMPTY && state.board[7][3]==EMPTY &&
                !isSquareAttacked(state, 7, 4, false) && !isSquareAttacked(state, 7, 3, false) && !isSquareAttacked(state, 7, 2, false)) {
                addMove(state, 7, 4, 7, 2, moves, EMPTY, false, true, false);
            }
        } else {
            if (state.blackKingSideCastle && state.board[0][5]==EMPTY && state.board[0][6]==EMPTY &&
                !isSquareAttacked(state, 0, 4, true) && !isSquareAttacked(state, 0, 5, true) && !isSquareAttacked(state, 0, 6, true)) {
                addMove(state, 0, 4, 0, 6, moves, EMPTY, true, false, false);
            }
            if (state.blackQueenSideCastle && state.board[0][1]==EMPTY && state.board[0][2]==EMPTY && state.board[0][3]==EMPTY &&
                !isSquareAttacked(state, 0, 4, true) && !isSquareAttacked(state, 0, 3, true) && !isSquareAttacked(state, 0, 2, true)) {
                addMove(state, 0, 4, 0, 2, moves, EMPTY, false, true, false);
            }
        }
    }
}

// Generate all pseudo-legal moves
void generateAllPseudoLegalMoves(const BoardState& state, std::vector<Move>& moves, bool capturesOnly) {
    moves.clear();
    const std::vector<std::pair<int, int>> R_DIRS = {{0,1},{0,-1},{1,0},{-1,0}};
    const std::vector<std::pair<int, int>> B_DIRS = {{1,1},{1,-1},{-1,1},{-1,-1}};
    std::vector<std::pair<int, int>> Q_DIRS = R_DIRS;
    Q_DIRS.insert(Q_DIRS.end(), B_DIRS.begin(), B_DIRS.end());
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = state.board[r][c];
            if (piece == EMPTY || (state.whiteToMove != isWhitePiece(piece))) continue;
            char upper_piece = toupper(piece);
            if (upper_piece == W_PAWN) generatePawnMoves(state, r, c, moves, capturesOnly);
            else if (upper_piece == W_KNIGHT) generateKnightMoves(state, r, c, moves, capturesOnly);
            else if (upper_piece == W_BISHOP) generateSlidingMoves(state, r, c, moves, B_DIRS, capturesOnly);
            else if (upper_piece == W_ROOK) generateSlidingMoves(state, r, c, moves, R_DIRS, capturesOnly);
            else if (upper_piece == W_QUEEN) generateSlidingMoves(state, r, c, moves, Q_DIRS, capturesOnly);
            else if (upper_piece == W_KING) generateKingMoves(state, r, c, moves, capturesOnly);
        }
    }
}

// Generate legal moves (filters out moves that leave king in check)
void generateLegalMoves(const BoardState& S, std::vector<Move>& legal_moves, bool capturesOnly) {
    legal_moves.clear();
    std::vector<Move> pseudo; generateAllPseudoLegalMoves(S, pseudo, capturesOnly);
    bool isWhite = S.whiteToMove;
    for (const auto& m : pseudo) {
        BoardState temp = S; apply_raw_move_to_board(temp, m);
        if (!isKingInCheck(temp, isWhite)) legal_moves.push_back(m);
    }
}

// MVV-LVA move ordering with killer moves and history heuristic
void orderMoves(const BoardState& state, std::vector<Move>& moves, int ply) {
    for (auto& move : moves) {
        move.score = 0;

        // 1. Captures (MVV-LVA) - highest priority
        if (move.isCapture(state)) {
            char movingPieceType = toupper(state.board[move.fromRow][move.fromCol]);
            char capturedPieceType;
            if (move.isEnPassantCapture) {
                capturedPieceType = W_PAWN;
            } else {
                capturedPieceType = toupper(state.board[move.toRow][move.toCol]);
            }

            int victimValue = 0;
            auto victim_it = mvv_lva_piece_values.find(capturedPieceType);
            if(victim_it != mvv_lva_piece_values.end()) victimValue = victim_it->second;

            int attackerValue = 10;
            auto attacker_it = mvv_lva_piece_values.find(movingPieceType);
            if(attacker_it != mvv_lva_piece_values.end()) attackerValue = attacker_it->second;

            move.score = (victimValue * 100) - attackerValue;
        }

        // 2. Promotions - very high priority
        if (move.promotionPiece != EMPTY) {
            auto promo_it = mvv_lva_piece_values.find(toupper(move.promotionPiece));
            if (promo_it != mvv_lva_piece_values.end()){
                 move.score += promo_it->second * 100;
            } else {
                 move.score += mvv_lva_piece_values.at(W_QUEEN) * 100;
            }
        }

        // 3. Killer moves (for quiet moves) - medium priority
        if (!move.isCapture(state) && move.promotionPiece == EMPTY && ply >= 0 && ply < MAX_SEARCH_PLY) {
            if (move == killerMoves[ply][0]) {
                move.score += KILLER_MOVE_1_SCORE;
            } else if (move == killerMoves[ply][1]) {
                move.score += KILLER_MOVE_2_SCORE;
            }

            // 4. History heuristic (for quiet moves) - lower priority
            int fromSquare = move.fromRow * 8 + move.fromCol;
            int toSquare = move.toRow * 8 + move.toCol;
            move.score += historyTable[fromSquare][toSquare] / HISTORY_SCORE_DIVISOR;
        }
    }

    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.score > b.score;
    });
}
