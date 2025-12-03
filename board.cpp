#include "board.h"
#include "constants.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>

// BoardState constructor
BoardState::BoardState() { reset(); }

// BoardState reset method
void BoardState::reset() {
    const char initial_board[8][8] = {
        {'r','n','b','q','k','b','n','r'}, {'p','p','p','p','p','p','p','p'},
        {EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY},{EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY},
        {EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY},{EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY},
        {'P','P','P','P','P','P','P','P'}, {'R','N','B','Q','K','B','N','R'}
    };
    for(int r=0; r<8; ++r) for(int c=0; c<8; ++c) board[r][c] = initial_board[r][c];
    whiteToMove = true;
    whiteKingSideCastle = whiteQueenSideCastle = true;
    blackKingSideCastle = blackQueenSideCastle = true;
    enPassantTarget = {-1,-1};
    halfmoveClock = 0; fullmoveNumber = 1;
    positionCounts.clear();
    currentFenKey = getPositionKey();
    addCurrentPositionToHistory();
}

// Get position key for repetition detection
std::string BoardState::getPositionKey() const {
    std::stringstream ss;
    for(int r=0; r<8; ++r) for(int c=0; c<8; ++c) ss << board[r][c];
    ss << (whiteToMove ? 'w' : 'b');
    ss << (whiteKingSideCastle ? 'K' : '-'); ss << (whiteQueenSideCastle ? 'Q' : '-');
    ss << (blackKingSideCastle ? 'k' : '-'); ss << (blackQueenSideCastle ? 'q' : '-');
    if (enPassantTarget.first != -1) ss << (char)('a'+enPassantTarget.second) << (char)('8'-enPassantTarget.first);
    else ss << '-';
    return ss.str();
}

// Add current position to history
void BoardState::addCurrentPositionToHistory() { positionCounts[currentFenKey]++; }

// Parse FEN string
void BoardState::parseFen(const std::string& fenStr) {
    std::fill(&board[0][0], &board[0][0]+sizeof(board), EMPTY);
    positionCounts.clear();
    std::istringstream fenStream(fenStr); std::string part;
    fenStream >> part; int r=0, c=0;
    for(char sym : part) {
        if(sym=='/') { r++; c=0; } else if(isdigit(sym)) { c+=(sym-'0'); }
        else { if(r<8 && c<8) board[r][c++] = sym; }
    }
    fenStream >> part; whiteToMove = (part=="w");
    fenStream >> part;
    whiteKingSideCastle = (part.find('K') != std::string::npos); whiteQueenSideCastle = (part.find('Q') != std::string::npos);
    blackKingSideCastle = (part.find('k') != std::string::npos); blackQueenSideCastle = (part.find('q') != std::string::npos);
    fenStream >> part;
    if(part=="-") enPassantTarget={-1,-1}; else { enPassantTarget = {'8'-part[1], part[0]-'a'}; }
    if(fenStream >> part) halfmoveClock=std::stoi(part); else halfmoveClock=0;
    if(fenStream >> part) fullmoveNumber=std::stoi(part); else fullmoveNumber=1;
    currentFenKey = getPositionKey();
    addCurrentPositionToHistory();
}

// Update FEN key
void BoardState::updateFenKey() {
    currentFenKey = getPositionKey();
}

// Helper functions
bool isSquareOnBoard(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }
char getPieceAt(const BoardState& state, int r, int c) { return isSquareOnBoard(r, c) ? state.board[r][c] : EMPTY; }
bool isWhitePiece(char piece) { return piece >= 'A' && piece <= 'Z'; }
bool isBlackPiece(char piece) { return piece >= 'a' && piece <= 'z'; }

// Move::isCapture implementation
bool Move::isCapture(const BoardState& state) const {
    return isEnPassantCapture || (getPieceAt(state, toRow, toCol) != EMPTY);
}

// Apply move to board state
void apply_raw_move_to_board(BoardState& state, const Move& move) {
    char piece = state.board[move.fromRow][move.fromCol];
    char captured = state.board[move.toRow][move.toCol];
    int ep_cap_row = state.whiteToMove ? move.toRow + 1 : move.toRow - 1;
    state.board[move.toRow][move.toCol] = piece;
    state.board[move.fromRow][move.fromCol] = EMPTY;
    if (move.promotionPiece != EMPTY) { state.board[move.toRow][move.toCol] = move.promotionPiece; }
    else if (move.isKingSideCastle) { state.board[move.fromRow][5] = state.board[move.fromRow][7]; state.board[move.fromRow][7] = EMPTY; }
    else if (move.isQueenSideCastle) { state.board[move.fromRow][3] = state.board[move.fromRow][0]; state.board[move.fromRow][0] = EMPTY; }
    else if (move.isEnPassantCapture) { state.board[ep_cap_row][move.toCol] = EMPTY; }
    state.enPassantTarget = {-1, -1};
    if (toupper(piece) == W_PAWN && abs(move.toRow - move.fromRow) == 2) { state.enPassantTarget = {(move.fromRow + move.toRow) / 2, move.fromCol}; }
    if (piece == W_KING) state.whiteKingSideCastle = state.whiteQueenSideCastle = false;
    else if (piece == B_KING) state.blackKingSideCastle = state.blackQueenSideCastle = false;
    else if (piece == W_ROOK) { if (move.fromRow == 7 && move.fromCol == 0) state.whiteQueenSideCastle = false; else if (move.fromRow == 7 && move.fromCol == 7) state.whiteKingSideCastle = false; }
    else if (piece == B_ROOK) { if (move.fromRow == 0 && move.fromCol == 0) state.blackQueenSideCastle = false; else if (move.fromRow == 0 && move.fromCol == 7) state.blackKingSideCastle = false; }
    if (captured == W_ROOK) { if (move.toRow == 7 && move.toCol == 0) state.whiteQueenSideCastle = false; else if (move.toRow == 7 && move.toCol == 7) state.whiteKingSideCastle = false; }
    else if (captured == B_ROOK) { if (move.toRow == 0 && move.toCol == 0) state.blackQueenSideCastle = false; else if (move.toRow == 0 && move.toCol == 7) state.blackKingSideCastle = false; }
    state.whiteToMove = !state.whiteToMove;
    state.updateFenKey();
}

// Check if square is attacked
bool isSquareAttacked(const BoardState& state, int r, int c, bool byWhiteAttacker) {
    int pawn_dir = byWhiteAttacker ? 1 : -1;
    char attacking_pawn = byWhiteAttacker ? W_PAWN : B_PAWN;
    if (getPieceAt(state, r + pawn_dir, c - 1) == attacking_pawn) return true;
    if (getPieceAt(state, r + pawn_dir, c + 1) == attacking_pawn) return true;
    char attacking_knight = byWhiteAttacker ? W_KNIGHT : B_KNIGHT;
    const std::vector<std::pair<int, int>> knight_deltas = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (auto d : knight_deltas) { if (getPieceAt(state, r + d.first, c + d.second) == attacking_knight) return true; }
    char attacking_rook = byWhiteAttacker ? W_ROOK : B_ROOK;
    char attacking_bishop = byWhiteAttacker ? W_BISHOP : B_BISHOP;
    char attacking_queen = byWhiteAttacker ? W_QUEEN : B_QUEEN;
    const std::vector<std::pair<int, int>> rook_dirs = {{0,1},{0,-1},{1,0},{-1,0}};
    const std::vector<std::pair<int, int>> bishop_dirs = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto dir : rook_dirs) {
        for (int i = 1; i < 8; ++i) {
            int nr=r+dir.first*i, nc=c+dir.second*i; if (!isSquareOnBoard(nr, nc)) break;
            char p=state.board[nr][nc]; if (p==attacking_rook || p==attacking_queen) return true; if (p!=EMPTY) break;
        }
    }
    for (auto dir : bishop_dirs) {
        for (int i = 1; i < 8; ++i) {
            int nr=r+dir.first*i, nc=c+dir.second*i; if (!isSquareOnBoard(nr, nc)) break;
            char p=state.board[nr][nc]; if (p==attacking_bishop || p==attacking_queen) return true; if (p!=EMPTY) break;
        }
    }
    char attacking_king = byWhiteAttacker ? W_KING : B_KING;
    const std::vector<std::pair<int, int>> king_deltas = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    for (auto d : king_deltas) { if (getPieceAt(state, r + d.first, c + d.second) == attacking_king) return true; }
    return false;
}

// Check if king is in check
bool isKingInCheck(const BoardState& state, bool kingIsWhite) {
    int kr = -1, kc = -1; char k_char = kingIsWhite ? W_KING : B_KING;
    for (int r = 0; r < 8 && kr == -1; ++r) { for (int c = 0; c < 8; ++c) { if (state.board[r][c] == k_char) { kr = r; kc = c; break; } } }
    return (kr != -1) && isSquareAttacked(state, kr, kc, !kingIsWhite);
}
