#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include <chrono>
#include <atomic>
#include <map>

// Global search state
extern std::atomic<bool> time_is_up;
extern std::atomic<uint64_t> nodes_searched;
extern std::map<std::string, TTEntry> transpositionTable;

// Search functions
int alphaBetaSearch(BoardState state, int depth, int alpha, int beta, bool maximizingPlayer,
                    const std::chrono::steady_clock::time_point& startTime,
                    const std::chrono::milliseconds& timeLimit);

int quiescenceSearch(BoardState state, int alpha, int beta, bool maximizingPlayer,
                     const std::chrono::steady_clock::time_point& startTime,
                     const std::chrono::milliseconds& timeLimit, int quiescenceDepth);

#endif // SEARCH_H
