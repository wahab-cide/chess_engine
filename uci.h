#ifndef UCI_H
#define UCI_H

#include "types.h"
#include <sstream>
#include <random>

// Global board state and RNG
extern BoardState currentBoard;
extern std::mt19937 global_rng;

// UCI handlers
void handleUci();
void handleIsReady();
void handleUciNewGame();
void handlePosition(std::istringstream& iss);
void handleGo(std::istringstream& iss);

// Game state queries
void master_apply_move(const Move& move);
bool isCheckmate();
bool isStalemate();
bool isThreefoldRepetition();
bool isFiftyMoveDraw();
std::string checkGameEndStatus();

#endif // UCI_H
