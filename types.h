#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <map>
#include <utility>
#include "constants.h"

struct BoardState; // Forward declaration

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    char promotionPiece;
    bool isKingSideCastle;
    bool isQueenSideCastle;
    bool isEnPassantCapture;
    int score;

    Move(int fr=0, int fc=0, int tr=0, int tc=0, char promo=EMPTY, bool ksc=false, bool qsc=false, bool ep=false)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), promotionPiece(promo),
          isKingSideCastle(ksc), isQueenSideCastle(qsc), isEnPassantCapture(ep), score(0) {}

    std::string toUci() const {
        std::string uci = "";
        uci += (char)('a' + fromCol); uci += (char)('8' - fromRow);
        uci += (char)('a' + toCol); uci += (char)('8' - toRow);
        if (promotionPiece != EMPTY) uci += tolower(promotionPiece);
        return uci;
    }

    bool operator==(const Move& other) const {
        return fromRow==other.fromRow && fromCol==other.fromCol && toRow==other.toRow &&
               toCol==other.toCol && promotionPiece==other.promotionPiece &&
               isKingSideCastle==other.isKingSideCastle && isQueenSideCastle==other.isQueenSideCastle &&
               isEnPassantCapture==other.isEnPassantCapture;
    }

    bool isCapture(const BoardState& state) const;
};

struct BoardState {
    char board[8][8];
    bool whiteToMove;
    bool whiteKingSideCastle, whiteQueenSideCastle;
    bool blackKingSideCastle, blackQueenSideCastle;
    std::pair<int, int> enPassantTarget;
    int halfmoveClock;
    int fullmoveNumber;
    std::map<std::string, int> positionCounts;
    std::string currentFenKey;

    BoardState();
    void reset();
    std::string getPositionKey() const;
    void addCurrentPositionToHistory();
    void parseFen(const std::string& fenStr);
    void updateFenKey();
};

#endif // TYPES_H
