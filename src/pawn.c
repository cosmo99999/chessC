#include "helpers.h"
#include "move.h"
#include <stdint.h>
#include <strings.h>

void pawn_moves(MoveArr *mArr, Position *position) {
  uint64_t pawns = 0;
  uint64_t enemyMask = 0;
  bool whitesMove;

  if (position->whitesMove) {
    whitesMove = true;
    pawns = position->wpawns;
    enemyMask = black_piece_mask(position);
  } else {
    whitesMove = false;
    pawns = position->bpawns;
    enemyMask = white_piece_mask(position);
  }

  if (whitesMove) {
    while (pawns) {
      uint64_t p = pop_lsb(&pawns);
      uint64_t attack = wpawn_attack_mask(&p);
      uint64_t enemyIncludingEnPassant = enemyMask | position->enPassantTile;
      attack ^= enemyIncludingEnPassant;
      while (attack) {
        uint64_t toPosition = pop_lsb(&attack);
        Piece toPiece = find_piece_type(toPosition, position);
        Move m = {p, toPosition, Pawn, toPiece, false, false, false};
        mArr->count++;
        mArr->moves[mArr->count] = m;
      }
      uint64_t moves = wpawn_move_mask(&p, enemyMask);
      while (moves) {
        uint64_t toPosition = pop_lsb(&moves);
        Move m = {p, toPosition, Pawn, None, false, false, false};
        mArr->count++;
        mArr->moves[mArr->count] = m;
      }
    }
  }
}
uint64_t wpawn_attack_mask(uint64_t *pawn) {
  uint64_t result = 0;
  uint64_t p = *pawn;
  if (!(p & A_FILE)) {
    result |= p << 7;
  }
  if (!(p & H_FILE)) {
    result |= p << 9;
  }
  return result;
}
uint64_t wpawn_move_mask(uint64_t *pawn, uint64_t enemyPos) {
  uint64_t result = 0;
  uint64_t p = *pawn;
  uint64_t firstPos = p << 8;
  uint64_t secondPos = p << 16;
  if (!(firstPos & enemyPos)) {
    result |= firstPos;
    if (p & ROW_ONE) {
      if (!(secondPos & enemyPos)) {
        result |= secondPos;
      }
    }
  }
  return result;
}
uint64_t bpawn_attack_mask(uint64_t *pawn) {
  uint64_t result = 0;
  uint64_t p = *pawn;
  if (!(p & A_FILE)) {
    result |= p >> 7;
  }
  if (!(p & H_FILE)) {
    result |= p >> 9;
  }
  return result;
}
uint64_t bpawn_move_mask(uint64_t *pawn, uint64_t enemyPos) {
  uint64_t result = 0;
  uint64_t p = *pawn;
  uint64_t firstPos = p >> 8;
  uint64_t secondPos = p >> 16;
  if (!(firstPos & enemyPos)) {
    result |= firstPos;
    if (p & ROW_SEVEN) {
      if (!(secondPos & enemyPos)) {
        result |= secondPos;
      }
    }
  }
  return result;
}
