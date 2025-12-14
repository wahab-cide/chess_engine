#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <map>

// Piece character constants
const char EMPTY = ' ';
const char W_PAWN = 'P', W_KNIGHT = 'N', W_BISHOP = 'B', W_ROOK = 'R', W_QUEEN = 'Q', W_KING = 'K';
const char B_PAWN = 'p', B_KNIGHT = 'n', B_BISHOP = 'b', B_ROOK = 'r', B_QUEEN = 'q', B_KING = 'k';

// Piece values for material evaluation (in centipawns)
const std::map<char, int> piece_values = {
    {W_PAWN, 100}, {B_PAWN, 100},
    {W_KNIGHT, 320}, {B_KNIGHT, 320},
    {W_BISHOP, 330}, {B_BISHOP, 330},
    {W_ROOK, 500}, {B_ROOK, 500},
    {W_QUEEN, 900}, {B_QUEEN, 900},
    {W_KING, 20000}, {B_KING, 20000}
};

// Simplified piece values for MVV-LVA (less granularity needed)
const std::map<char, int> mvv_lva_piece_values = {
    {W_PAWN, 1}, {B_PAWN, 1},
    {W_KNIGHT, 3}, {B_KNIGHT, 3},
    {W_BISHOP, 3}, {B_BISHOP, 3},
    {W_ROOK, 5}, {B_ROOK, 5},
    {W_QUEEN, 9}, {B_QUEEN, 9},
    {W_KING, 10}, {B_KING, 10}
};

// Piece-Square Tables (PSTs)
const int pawn_pst[64] = {
    0,0,0,0,0,0,0,0,
    50,50,50,50,50,50,50,50,
    10,10,20,30,30,20,10,10,
    5,5,10,25,25,10,5,5,
    0,0,0,20,20,0,0,0,
    5,-5,-10,0,0,-10,-5,5,
    5,10,10,-20,-20,10,10,5,
    0,0,0,0,0,0,0,0
};

const int knight_pst[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,0,0,0,0,-20,-40,
    -30,0,10,15,15,10,0,-30,
    -30,5,15,20,20,15,5,-30,
    -30,0,15,20,20,15,0,-30,
    -30,5,10,15,15,10,5,-30,
    -40,-20,0,5,5,0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishop_pst[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,0,0,0,0,0,0,-10,
    -10,0,5,10,10,5,0,-10,
    -10,5,5,10,10,5,5,-10,
    -10,0,10,10,10,10,0,-10,
    -10,10,10,10,10,10,10,-10,
    -10,5,0,0,0,0,5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int rook_pst[64] = {
    0,0,0,0,0,0,0,0,
    5,10,10,10,10,10,10,5,
    -5,0,0,0,0,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    -5,0,0,0,0,0,0,-5,
    0,0,0,5,5,0,0,0
};

const int queen_pst[64] = {
    -20,-10,-10,-5,-5,-10,-10,-20,
    -10,0,0,0,0,0,0,-10,
    -10,0,5,5,5,5,0,-10,
    -5,0,5,5,5,5,0,-5,
    0,0,5,5,5,5,0,-5,
    -10,5,5,5,5,5,0,-10,
    -10,0,5,0,0,0,0,-10,
    -20,-10,-10,-5,-5,-10,-10,-20
};

const int king_pst_mg[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20,20,0,0,0,0,20,20,
    20,30,10,0,0,10,30,20
};

const int king_pst_eg[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,0,0,-10,-20,-30,
    -30,-10,20,30,30,20,-10,-30,
    -30,-10,30,40,40,30,-10,-30,
    -30,-10,30,40,40,30,-10,-30,
    -30,-10,20,30,30,20,-10,-30,
    -30,-30,0,0,0,0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

// Evaluation scores for terminal states
const int MATE_SCORE = 100000;
const int DRAW_SCORE = 0;
const int MAX_SEARCH_PLY = 64;
const int MAX_QUIESCENCE_PLY = 6;
const int IN_CHECK_PENALTY = 50;
const int LMR_REDUCTION = 1;
const int LMR_MIN_MOVES_TO_TRY_REDUCTION = 3;
const int LMR_MIN_DEPTH_FOR_REDUCTION = 3;
const int CHECK_EXTENSION_PLY = 1;

// Transposition Table
const size_t MAX_TT_SIZE = 1000000;

// Null Move Pruning
const int NULL_MOVE_REDUCTION = 2;
const int NULL_MOVE_MIN_DEPTH = 3;

// Killer Moves (2 killer moves per ply)
const int NUM_KILLER_MOVES = 2;

// Move Ordering Scores
const int KILLER_MOVE_1_SCORE = 900;
const int KILLER_MOVE_2_SCORE = 800;
const int HISTORY_SCORE_DIVISOR = 100;

// Aspiration Windows
const int ASPIRATION_WINDOW = 50;
const int ASPIRATION_MIN_DEPTH = 3;

// Transposition Table Entry Flags
enum TTEntryFlag { TT_EXACT, TT_LOWERBOUND, TT_UPPERBOUND, TT_INVALID };

struct TTEntry {
    int score;
    int depth;
    TTEntryFlag flag;

    TTEntry() : score(0), depth(-1), flag(TT_INVALID) {}
};

#endif // CONSTANTS_H
