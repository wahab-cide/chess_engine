#include "evaluation.h"
#include "board.h"
#include "constants.h"
#include <cctype>

// Evaluate the board position from white's perspective
int evaluateBoard(const BoardState& state) {
    int score = 0;
    int total_material_no_kings = 0;

    for(int r=0; r<8; ++r) {
        for(int c=0; c<8; ++c) {
            char piece = state.board[r][c];
            if(piece!=EMPTY) {
                auto it = piece_values.find(piece);
                int piece_val = 0;
                if(it != piece_values.end()) piece_val = it->second;

                if (toupper(piece) != W_KING) {
                    auto mat_it = piece_values.find(toupper(piece));
                    if (mat_it != piece_values.end()) total_material_no_kings += mat_it->second;
                }

                int pst_bonus = 0;
                int square_index = r * 8 + c;
                int black_square_index = (7 - r) * 8 + c;

                if (isWhitePiece(piece)) {
                    score += piece_val;
                    if (piece == W_PAWN) pst_bonus = pawn_pst[square_index];
                    else if (piece == W_KNIGHT) pst_bonus = knight_pst[square_index];
                    else if (piece == W_BISHOP) pst_bonus = bishop_pst[square_index];
                    else if (piece == W_ROOK) pst_bonus = rook_pst[square_index];
                    else if (piece == W_QUEEN) pst_bonus = queen_pst[square_index];
                    else if (piece == W_KING) {
                        if (total_material_no_kings < 1500) pst_bonus = king_pst_eg[square_index];
                        else pst_bonus = king_pst_mg[square_index];
                    }
                    score += pst_bonus;
                } else {
                    score -= piece_val;
                    if (piece == B_PAWN) pst_bonus = pawn_pst[black_square_index];
                    else if (piece == B_KNIGHT) pst_bonus = knight_pst[black_square_index];
                    else if (piece == B_BISHOP) pst_bonus = bishop_pst[black_square_index];
                    else if (piece == B_ROOK) pst_bonus = rook_pst[black_square_index];
                    else if (piece == B_QUEEN) pst_bonus = queen_pst[black_square_index];
                    else if (piece == B_KING) {
                        if (total_material_no_kings < 1500) pst_bonus = king_pst_eg[black_square_index];
                        else pst_bonus = king_pst_mg[black_square_index];
                    }
                    score -= pst_bonus;
                }
            }
        }
    }
    return score;
}
