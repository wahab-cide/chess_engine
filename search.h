#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include <chrono>
#include <atomic>
#include <map>
#include <vector>

// Global search state
extern std::atomic<bool> time_is_up;
extern std::atomic<uint64_t> nodes_searched;
extern std::map<std::string, TTEntry> transpositionTable;

// Killer moves: [ply][killer_index]
extern Move killerMoves[64][2];

// History heuristic: [from_square][to_square]
extern int historyTable[64][64];

// Helper functions
void clearKillerMoves();
void clearHistoryTable();

// Search functions
int alphaBetaSearch(BoardState state, int depth, int alpha, int beta, bool maximizingPlayer,
                    const std::chrono::steady_clock::time_point& startTime,
                    const std::chrono::milliseconds& timeLimit, int ply, bool allowNullMove);

int quiescenceSearch(BoardState state, int alpha, int beta, bool maximizingPlayer,
                     const std::chrono::steady_clock::time_point& startTime,
                     const std::chrono::milliseconds& timeLimit, int quiescenceDepth);

#endif // SEARCH_H
