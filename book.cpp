#include "book.h"
#include "board.h"
#include "movegen.h"
#include "constants.h"
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <iostream>

// Global opening book instance
OpeningBook globalBook;

// External RNG (defined in uci.cpp)
extern std::mt19937 global_rng;

OpeningBook::OpeningBook() {
    // Constructor - book starts empty
}

// Get simplified position key for book lookup
// Format: board position + side to move + castling rights + en passant
// Excludes halfmove and fullmove counters (they don't affect position)
std::string OpeningBook::getBookKey(const BoardState& state) const {
    std::string key;

    // Board position (8 ranks)
    for (int r = 0; r < 8; ++r) {
        int emptyCount = 0;
        for (int c = 0; c < 8; ++c) {
            char piece = state.board[r][c];
            if (piece == EMPTY) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    key += std::to_string(emptyCount);
                    emptyCount = 0;
                }
                key += piece;
            }
        }
        if (emptyCount > 0) {
            key += std::to_string(emptyCount);
        }
        if (r < 7) key += '/';
    }

    // Side to move
    key += ' ';
    key += state.whiteToMove ? 'w' : 'b';

    // Castling rights
    key += ' ';
    std::string castling;
    if (state.whiteKingSideCastle) castling += 'K';
    if (state.whiteQueenSideCastle) castling += 'Q';
    if (state.blackKingSideCastle) castling += 'k';
    if (state.blackQueenSideCastle) castling += 'q';
    if (castling.empty()) castling = "-";
    key += castling;

    // En passant target
    key += ' ';
    if (state.enPassantTarget.first == -1) {
        key += '-';
    } else {
        key += (char)('a' + state.enPassantTarget.second);
        key += (char)('8' - state.enPassantTarget.first);
    }

    return key;
}

// Parse a single line from the book file
// Format: "FEN,move1,move2,move3,..."
void OpeningBook::parseBookLine(const std::string& line) {
    if (line.empty() || line[0] == '#') return; // Skip empty lines and comments

    // Find the first comma (separates FEN from moves)
    size_t commaPos = line.find(',');
    if (commaPos == std::string::npos) return; // Invalid line

    std::string fen = line.substr(0, commaPos);
    std::string movesStr = line.substr(commaPos + 1);

    // Parse FEN to get book key
    BoardState tempState;
    tempState.parseFen(fen);
    std::string key = getBookKey(tempState);

    // Parse moves (comma-separated)
    std::vector<std::string> moves;
    std::istringstream iss(movesStr);
    std::string move;
    while (std::getline(iss, move, ',')) {
        // Trim whitespace
        move.erase(0, move.find_first_not_of(" \t\r\n"));
        move.erase(move.find_last_not_of(" \t\r\n") + 1);
        if (!move.empty()) {
            moves.push_back(move);
        }
    }

    // Add to book (append if key already exists)
    if (!moves.empty()) {
        if (bookMoves.find(key) == bookMoves.end()) {
            bookMoves[key] = moves;
        } else {
            // Append new moves to existing ones
            bookMoves[key].insert(bookMoves[key].end(), moves.begin(), moves.end());
        }
    }
}

// Load opening book from file
bool OpeningBook::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open opening book file: " << filename << std::endl;
        return false;
    }

    clear(); // Clear existing book

    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        parseBookLine(line);
        lineCount++;
    }

    file.close();

    std::cerr << "Loaded opening book: " << size() << " positions from "
              << lineCount << " lines in " << filename << std::endl;
    return true;
}

// Probe the book for a move
bool OpeningBook::probeBook(const BoardState& state, Move& move) const {
    std::string key = getBookKey(state);

    auto it = bookMoves.find(key);
    if (it == bookMoves.end()) {
        return false; // Position not in book
    }

    const std::vector<std::string>& moves = it->second;
    if (moves.empty()) {
        return false; // No moves for this position
    }

    // Generate legal moves to validate book move
    std::vector<Move> legalMoves;
    generateLegalMoves(state, legalMoves, false);
    if (legalMoves.empty()) {
        return false; // No legal moves (shouldn't happen)
    }

    // Try to find a valid book move
    // Shuffle the book moves for variety
    std::vector<std::string> shuffledMoves = moves;
    std::shuffle(shuffledMoves.begin(), shuffledMoves.end(), global_rng);

    for (const std::string& bookMoveStr : shuffledMoves) {
        // Parse UCI move string (e.g., "e2e4", "e7e8q")
        if (bookMoveStr.length() < 4) continue;

        int fromCol = bookMoveStr[0] - 'a';
        int fromRow = '8' - bookMoveStr[1];
        int toCol = bookMoveStr[2] - 'a';
        int toRow = '8' - bookMoveStr[3];

        char promo = EMPTY;
        if (bookMoveStr.length() == 5) {
            char promoChar = bookMoveStr[4];
            char pieceColor = state.whiteToMove ? 'W' : 'B';
            if (promoChar == 'q') promo = (pieceColor == 'W' ? W_QUEEN : B_QUEEN);
            else if (promoChar == 'r') promo = (pieceColor == 'W' ? W_ROOK : B_ROOK);
            else if (promoChar == 'b') promo = (pieceColor == 'W' ? W_BISHOP : B_BISHOP);
            else if (promoChar == 'n') promo = (pieceColor == 'W' ? W_KNIGHT : B_KNIGHT);
        }

        // Find matching legal move
        for (const Move& legalMove : legalMoves) {
            if (legalMove.fromRow == fromRow && legalMove.fromCol == fromCol &&
                legalMove.toRow == toRow && legalMove.toCol == toCol &&
                legalMove.promotionPiece == promo) {
                move = legalMove;
                return true; // Found valid book move
            }
        }
    }

    return false; // No valid book move found
}
