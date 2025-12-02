#ifndef BOARD_H
#define BOARD_H

#include "types.h"

// Helper functions
bool isSquareOnBoard(int r, int c);
char getPieceAt(const BoardState& state, int r, int c);
bool isWhitePiece(char piece);
bool isBlackPiece(char piece);

// Board state manipulation
void apply_raw_move_to_board(BoardState& state, const Move& move);

// Check detection
bool isSquareAttacked(const BoardState& state, int r, int c, bool byWhiteAttacker);
bool isKingInCheck(const BoardState& state, bool kingIsWhite);

#endif // BOARD_H
