#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  Pawn,
  Knight,
  Bishop,
  Rook,
  Queen,
  King,
  None,
} Piece;

typedef enum {
  White,
  Black,
  NoColour,
} Colour;

typedef struct {
  uint64_t from;
  uint64_t to;
  Piece pfrom;
  Piece pto;
  bool castling;
  bool promotion;
  Piece promotionPiece;
} Move;

typedef struct {
  int fullmoves;
  int halfmoves;
  uint64_t prevHash;
  uint64_t castlingRights;
  uint64_t enPassantTile;
} UndoMove;

typedef struct {
  size_t count;
  Move moves[300];
} MoveArr;

typedef struct {
  uint64_t hashes[500];
  size_t count;
} HashHistory;

typedef struct {
  int pieceKeys[2][6][64];
  int castlingKeys[16];
  int enPassantKeys[8];
  uint64_t sideToMoveKey;
} Zobrist;

typedef struct {
  uint64_t zhash;
  uint64_t wpawns;
  uint64_t wknights;
  uint64_t wbishops;
  uint64_t wqueens;
  uint64_t wrooks;
  uint64_t wking;

  uint64_t bpawns;
  uint64_t bknights;
  uint64_t bbishops;
  uint64_t bqueens;
  uint64_t brooks;
  uint64_t bking;
  // 1 == whiteKS 2 == whiteQS 4 == blackKS 8 == blackQS
  uint8_t castlingRights;
  uint64_t enPassantTile;
  bool whitesMove;
  int halfMoveClock;
  int fullMoveClock;

} Position;

// hashing
Zobrist init_zobrist();
uint64_t compute_zobrist(Position *position);
Position start_position();
UndoMove make_move(Move *move, Position *position);
void unmake_move(Move *move, Position *position, UndoMove *undo);
uint64_t white_piece_mask(Position *position);
uint64_t black_piece_mask(Position *position);
