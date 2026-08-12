#include "helpers.h"
#include "move.h"
#include <stdint.h>
#include <strings.h>

bool is_promotion(uint64_t from, uint64_t to) {
  if ((from & ROW_SEVEN) && (to & ROW_EIGHT)) {
    return true;
  }
  if ((from & ROW_TWO) && (to & ROW_ONE)) {
    return true;
  }
  return false;
}

void pawn_moves(MoveArr *mArr, Position *position) {
  uint64_t pawns = 0;
  uint64_t enemyMask = 0;
  uint64_t friendlyMask = 0;
  uint64_t allOccupancy = 0;

  if (position->whitesMove) {
    pawns = position->wpawns;
    enemyMask = black_piece_mask(position);
    friendlyMask = white_piece_mask(position);
  } else {
    pawns = position->bpawns;
    enemyMask = white_piece_mask(position);
    friendlyMask = black_piece_mask(position);
  }
  allOccupancy = friendlyMask | enemyMask;

  while (pawns) {
    uint64_t p = pop_lsb(&pawns);
    uint64_t attack = 0;
    if (position->whitesMove)
      attack = wpawn_attack_mask(&p);
    else
      attack = bpawn_attack_mask(&p);
    uint64_t enemyIncludingEnPassant = enemyMask | position->enPassantTile;
    attack &= enemyIncludingEnPassant;

    while (attack) {
      uint64_t toPosition = pop_lsb(&attack);
      Piece toPiece = find_piece_type(toPosition, position);
      if (is_promotion(p, toPosition)) {
        // 2 knight, 3 bishop, 4 rook, 5 queen
        for (int i = 2; i < 6; i++) {
          Move m = {p, toPosition, Pawn, toPiece, false, true, i};
          mArr->moves[mArr->count] = m;
          mArr->count++;
        }
      } else {
        Move m = {p, toPosition, Pawn, toPiece, false, false, None};
        mArr->moves[mArr->count] = m;
        mArr->count++;
      }
    }
    uint64_t moves = 0;
    if (position->whitesMove)
      moves = wpawn_move_mask(&p, allOccupancy);
    else
      moves = bpawn_move_mask(&p, allOccupancy);
    while (moves) {
      uint64_t toPosition = pop_lsb(&moves);
      if (is_promotion(p, toPosition)) {
        // 2 knight, 3 bishop, 4 rook, 5 queen
        for (int i = 2; i < 6; i++) {
          Move m = {p, toPosition, Pawn, None, false, true, i};
          mArr->moves[mArr->count] = m;
          mArr->count++;
        }
      } else {
        Move m = {p, toPosition, Pawn, None, false, false, None};
        mArr->moves[mArr->count] = m;
        mArr->count++;
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
uint64_t wpawn_move_mask(uint64_t *pawn, uint64_t allOccupancy) {
  uint64_t result = 0;
  uint64_t p = *pawn;
  uint64_t firstPos = p << 8;
  uint64_t secondPos = p << 16;
  if (!(firstPos & allOccupancy)) {
    result |= firstPos;
    if (p & ROW_TWO) {
      if (!(secondPos & allOccupancy)) {
        result |= secondPos;
      }
    }
  }

  return result;
}
uint64_t bpawn_attack_mask(uint64_t *pawn) {
  uint64_t result = 0;
  uint64_t p = *pawn;
  if (!(p & H_FILE)) {
    result |= p >> 7;
  }
  if (!(p & A_FILE)) {
    result |= p >> 9;
  }
  return result;
}
uint64_t bpawn_move_mask(uint64_t *pawn, uint64_t allOccupancy) {
  uint64_t result = 0;
  uint64_t p = *pawn;
  uint64_t firstPos = p >> 8;
  uint64_t secondPos = p >> 16;
  if (!(firstPos & allOccupancy)) {
    result |= firstPos;
    if (p & ROW_SEVEN) {
      if (!(secondPos & allOccupancy)) {
        result |= secondPos;
      }
    }
  }
  return result;
}
