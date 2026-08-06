#include "helpers.h"
#include "move.h"
#include <stdint.h>

#ifndef KNIGHT_RESTRICTIONS
#define KNIGHT_RESTRICTIONS

#define UP_17 ROW_SEVEN | ROW_EIGHT | H_FILE
#define DOWN_17 ROW_ONE | ROW_TWO | A_FILE
#define UP_15 ROW_SEVEN | ROW_EIGHT | A_FILE
#define DOWN_15 ROW_ONE | ROW_TWO | H_FILE
#define UP_6 A_FILE | B_FILE | ROW_EIGHT
#define DOWN_6 G_FILE | H_FILE | ROW_ONE
#define UP_10 G_FILE | H_FILE | ROW_EIGHT
#define DOWN_10 A_FILE | B_FILE | ROW_ONE

#endif

void knight_moves(MoveArr *mArr, Position *position) {
  uint64_t knights = 0;

  if (position->whitesMove) {
    knights = position->wknights;
  } else {
    knights = position->bknights;
  }
  while (knights) {
    uint64_t k = pop_lsb(&knights);
    uint64_t attack = knight_attack_mask(&k);
    while (attack) {
      uint64_t toPosition = pop_lsb(&attack);
      Piece toPiece = find_piece_type(toPosition, position);
      Move m = {k, toPosition, Knight, toPiece, false, false, false};
      mArr->count++;
      mArr->moves[mArr->count] = m;
    }
  }
}

uint64_t knight_attack_mask(uint64_t *knight) {
  uint64_t k = *knight;
  uint64_t attack = 0;

  if (!(k & UP_17))
    attack |= k << 17;
  if (!(k & DOWN_17))
    attack |= k >> 17;
  if (!(k & UP_15))
    attack |= k << 15;
  if (!(k & DOWN_15))
    attack |= k >> 15;
  if (!(k & UP_6))
    attack |= k << 6;
  if (!(k & DOWN_6))
    attack |= k >> 6;
  if (!(k & UP_10))
    attack |= k << 10;
  if (!(k & DOWN_10))
    attack |= k >> 10;

  return attack;
}
