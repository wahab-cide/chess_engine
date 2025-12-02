#ifndef BOOK_H
#define BOOK_H

#include "types.h"
#include <string>
#include <vector>
#include <map>

// Opening Book System
// Simple text-based format: Each line is "FEN,move1,move2,move3"
// Supports multiple moves per position for variety

class OpeningBook {
private:
    // Map from position key (simplified FEN) to vector of book moves
    std::map<std::string, std::vector<std::string>> bookMoves;

    // Helper function to normalize position key (FEN without move counters)
    std::string getBookKey(const BoardState& state) const;

    // Helper function to parse a book line
    void parseBookLine(const std::string& line);

public:
    OpeningBook();

    // Load opening book from file
    // Returns true if successful, false otherwise
    bool loadFromFile(const std::string& filename);

    // Probe the book for a move in the current position
    // Returns true and sets 'move' if book move found
    // Returns false if position not in book
    bool probeBook(const BoardState& state, Move& move) const;

    // Get number of positions in book
    int size() const { return bookMoves.size(); }

    // Clear the book
    void clear() { bookMoves.clear(); }
};

// Global opening book instance
extern OpeningBook globalBook;

#endif // BOOK_H
