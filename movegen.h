#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include <vector>

// Move generation
void generateLegalMoves(const BoardState& state, std::vector<Move>& legal_moves, bool capturesOnly = false);
void generateAllPseudoLegalMoves(const BoardState& state, std::vector<Move>& moves, bool capturesOnly);
void orderMoves(const BoardState& state, std::vector<Move>& moves);

// Helper for adding moves
void addMove(const BoardState& s, int r1, int c1, int r2, int c2, std::vector<Move>& m,
             char promo=EMPTY, bool ksc=false, bool qsc=false, bool ep=false);

// Piece-specific move generation
void generatePawnMoves(const BoardState& state, int r, int c, std::vector<Move>& moves, bool capturesOnly);
void generateKnightMoves(const BoardState& state, int r, int c, std::vector<Move>& moves, bool capturesOnly);
void generateSlidingMoves(const BoardState& state, int r, int c, std::vector<Move>& moves,
                         const std::vector<std::pair<int, int>>& directions, bool capturesOnly);
void generateKingMoves(const BoardState& state, int r, int c, std::vector<Move>& moves, bool capturesOnly);

#endif // MOVEGEN_H
