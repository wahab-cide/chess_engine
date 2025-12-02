#include "pawn_structure.h"
#include "board.h"
#include "constants.h"

// Main pawn structure evaluation function
int evaluatePawnStructure(const BoardState& state) {
    int score = 0;

    score += evaluateDoubledPawns(state);
    score += evaluateIsolatedPawns(state);
    score += evaluatePassedPawns(state);
    score += evaluateConnectedPawns(state);
    score += evaluateBackwardPawns(state);
    score += evaluatePawnChains(state);

    return score;
}

// Evaluate doubled pawns (penalty for having multiple pawns on same file)
int evaluateDoubledPawns(const BoardState& state) {
    int score = 0;

    for (int col = 0; col < 8; col++) {
        int whitePawns = 0;
        int blackPawns = 0;

        // Count pawns on this file
        for (int row = 0; row < 8; row++) {
            if (state.board[row][col] == W_PAWN) whitePawns++;
            if (state.board[row][col] == B_PAWN) blackPawns++;
        }

        // Penalize doubled (and tripled!) pawns
        if (whitePawns > 1) {
            score += DOUBLED_PAWN_PENALTY * (whitePawns - 1);
        }
        if (blackPawns > 1) {
            score -= DOUBLED_PAWN_PENALTY * (blackPawns - 1);
        }
    }

    return score;
}

// Check if a pawn is isolated (no friendly pawns on adjacent files)
bool isIsolatedPawn(const BoardState& state, int row, int col, bool isWhite) {
    char pawn = isWhite ? W_PAWN : B_PAWN;

    // Check left and right adjacent files for friendly pawns
    for (int adjacentCol : {col - 1, col + 1}) {
        if (adjacentCol < 0 || adjacentCol >= 8) continue;

        for (int r = 0; r < 8; r++) {
            if (state.board[r][adjacentCol] == pawn) {
                return false; // Found a friendly pawn on adjacent file
            }
        }
    }

    return true; // No friendly pawns on adjacent files
}

// Evaluate isolated pawns
int evaluateIsolatedPawns(const BoardState& state) {
    int score = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = state.board[row][col];

            if (piece == W_PAWN && isIsolatedPawn(state, row, col, true)) {
                score += ISOLATED_PAWN_PENALTY;
            }
            if (piece == B_PAWN && isIsolatedPawn(state, row, col, false)) {
                score -= ISOLATED_PAWN_PENALTY;
            }
        }
    }

    return score;
}

// Check if a pawn is passed (no enemy pawns blocking its path to promotion)
bool isPassedPawn(const BoardState& state, int row, int col, bool isWhite) {
    char enemyPawn = isWhite ? B_PAWN : W_PAWN;
    int direction = isWhite ? -1 : 1; // White pawns move up (row decreases), black down

    // Check the pawn's file and adjacent files ahead
    for (int checkCol = col - 1; checkCol <= col + 1; checkCol++) {
        if (checkCol < 0 || checkCol >= 8) continue;

        // Check all squares ahead on this file
        for (int checkRow = row + direction;
             checkRow >= 0 && checkRow < 8;
             checkRow += direction) {

            if (state.board[checkRow][checkCol] == enemyPawn) {
                return false; // Enemy pawn blocks the path
            }
        }
    }

    return true; // No enemy pawns blocking
}

// Evaluate passed pawns (bonus increases with advancement)
int evaluatePassedPawns(const BoardState& state) {
    int score = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = state.board[row][col];

            if (piece == W_PAWN && isPassedPawn(state, row, col, true)) {
                // White pawns: rank 0 is 8th rank, rank 7 is 1st rank
                int rank = 7 - row; // Convert to rank from white's perspective (0-7)
                int bonus = PASSED_PAWN_BASE + (rank * PASSED_PAWN_RANK_BONUS);
                score += bonus;
            }

            if (piece == B_PAWN && isPassedPawn(state, row, col, false)) {
                // Black pawns: rank 0 is 1st rank (from black's perspective)
                int rank = row; // Rank from black's perspective (0-7)
                int bonus = PASSED_PAWN_BASE + (rank * PASSED_PAWN_RANK_BONUS);
                score -= bonus;
            }
        }
    }

    return score;
}

// Check if a pawn has a connected (adjacent) friendly pawn
bool hasConnectedPawn(const BoardState& state, int row, int col, bool isWhite) {
    char pawn = isWhite ? W_PAWN : B_PAWN;

    // Check adjacent files on same row or one row away
    for (int dCol = -1; dCol <= 1; dCol += 2) { // -1 and +1
        int adjCol = col + dCol;
        if (adjCol < 0 || adjCol >= 8) continue;

        // Check same row and one row forward/backward
        for (int dRow = -1; dRow <= 1; dRow++) {
            int adjRow = row + dRow;
            if (adjRow < 0 || adjRow >= 8) continue;

            if (state.board[adjRow][adjCol] == pawn) {
                return true;
            }
        }
    }

    return false;
}

// Evaluate connected pawns
int evaluateConnectedPawns(const BoardState& state) {
    int score = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = state.board[row][col];

            if (piece == W_PAWN && hasConnectedPawn(state, row, col, true)) {
                score += CONNECTED_PAWN_BONUS;
            }
            if (piece == B_PAWN && hasConnectedPawn(state, row, col, false)) {
                score -= CONNECTED_PAWN_BONUS;
            }
        }
    }

    return score;
}

// Check if a pawn is backward (can't safely advance and is behind friendly pawns)
bool isBackwardPawn(const BoardState& state, int row, int col, bool isWhite) {
    char pawn = isWhite ? W_PAWN : B_PAWN;
    char enemyPawn = isWhite ? B_PAWN : W_PAWN;
    int direction = isWhite ? -1 : 1;

    // Check if pawn can safely advance
    int nextRow = row + direction;
    if (nextRow < 0 || nextRow >= 8) return false;

    // If square ahead is occupied by friendly piece, not backward
    if (state.board[nextRow][col] != EMPTY) {
        return false;
    }

    // Check if advancing would be safe (no enemy pawns attacking next square)
    for (int attackCol = col - 1; attackCol <= col + 1; attackCol += 2) {
        if (attackCol < 0 || attackCol >= 8) continue;

        int enemyRow = nextRow + direction;
        if (enemyRow >= 0 && enemyRow < 8) {
            if (state.board[enemyRow][attackCol] == enemyPawn) {
                // Pawn would be attacked if it advances
                // Now check if it's behind pawns on adjacent files
                for (int adjCol : {col - 1, col + 1}) {
                    if (adjCol < 0 || adjCol >= 8) continue;

                    // Look for friendly pawns ahead on adjacent files
                    for (int checkRow = row + direction;
                         checkRow >= 0 && checkRow < 8;
                         checkRow += direction) {

                        if (state.board[checkRow][adjCol] == pawn) {
                            return true; // Backward: can't advance and behind friendly pawns
                        }
                    }
                }
            }
        }
    }

    return false;
}

// Evaluate backward pawns
int evaluateBackwardPawns(const BoardState& state) {
    int score = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = state.board[row][col];

            if (piece == W_PAWN && isBackwardPawn(state, row, col, true)) {
                score += BACKWARD_PAWN_PENALTY;
            }
            if (piece == B_PAWN && isBackwardPawn(state, row, col, false)) {
                score -= BACKWARD_PAWN_PENALTY;
            }
        }
    }

    return score;
}

// Evaluate pawn chains (diagonal chains of pawns)
int evaluatePawnChains(const BoardState& state) {
    int score = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = state.board[row][col];

            if (piece == W_PAWN) {
                // Check if white pawn has supporting pawn diagonally behind
                for (int dCol = -1; dCol <= 1; dCol += 2) {
                    int supportCol = col + dCol;
                    int supportRow = row + 1; // Behind for white

                    if (supportRow < 8 && supportCol >= 0 && supportCol < 8) {
                        if (state.board[supportRow][supportCol] == W_PAWN) {
                            score += PAWN_CHAIN_BONUS;
                        }
                    }
                }
            }

            if (piece == B_PAWN) {
                // Check if black pawn has supporting pawn diagonally behind
                for (int dCol = -1; dCol <= 1; dCol += 2) {
                    int supportCol = col + dCol;
                    int supportRow = row - 1; // Behind for black

                    if (supportRow >= 0 && supportCol >= 0 && supportCol < 8) {
                        if (state.board[supportRow][supportCol] == B_PAWN) {
                            score -= PAWN_CHAIN_BONUS;
                        }
                    }
                }
            }
        }
    }

    return score;
}
