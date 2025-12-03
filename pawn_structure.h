#ifndef PAWN_STRUCTURE_H
#define PAWN_STRUCTURE_H

#include "types.h"

// Pawn structure evaluation constants (in centipawns)
const int DOUBLED_PAWN_PENALTY = -20;
const int ISOLATED_PAWN_PENALTY = -15;
const int PASSED_PAWN_BASE = 30;
const int PASSED_PAWN_RANK_BONUS = 15;
const int CONNECTED_PAWN_BONUS = 5;
const int BACKWARD_PAWN_PENALTY = -10;
const int PAWN_CHAIN_BONUS = 8;

// Main evaluation function
int evaluatePawnStructure(const BoardState& state);

// Individual pawn structure evaluations
int evaluateDoubledPawns(const BoardState& state);
int evaluateIsolatedPawns(const BoardState& state);
int evaluatePassedPawns(const BoardState& state);
int evaluateConnectedPawns(const BoardState& state);
int evaluateBackwardPawns(const BoardState& state);
int evaluatePawnChains(const BoardState& state);

// Helper functions
bool isPassedPawn(const BoardState& state, int row, int col, bool isWhite);
bool isIsolatedPawn(const BoardState& state, int col, bool isWhite);
bool isBackwardPawn(const BoardState& state, int row, int col, bool isWhite);
bool hasConnectedPawn(const BoardState& state, int row, int col, bool isWhite);

#endif // PAWN_STRUCTURE_H
