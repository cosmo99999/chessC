#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  None,
  Pawn,
  Knight,
  Bishop,
  Rook,
  Queen,
  King,
} Piece;

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
  size_t count;
  Move moves[300];
} MoveArr;

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

  uint8_t castlingRights;
  uint64_t enPassantTile;

  uint64_t checkingPiece;
  bool multiCheck;

  bool whitesMove;

  int halfMoveClock;
  int fullMoveClock;

} Position;

uint64_t white_piece_mask(Position *position);
uint64_t black_piece_mask(Position *position);
uint64_t *find_piece_mask(uint64_t mask, Position *position);
Piece find_piece_type(uint64_t mask, Position *position);
