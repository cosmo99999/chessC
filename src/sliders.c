#include "helpers.h"
#include "move.h"
#include "position.h"
#include <stdint.h>

void slider_moves(MoveArr *mArr, Position *position, Piece piece) {
  uint64_t pieces = 0;
  uint64_t allOccupancy = white_piece_mask(position) | black_piece_mask(position);
  uint64_t friendly = 0;
  int *offsets;
  int length = 0;

  if (position->whitesMove) {
    friendly = white_piece_mask(position);
    if (piece == Queen) {
      pieces = position->wqueens;
      offsets = QUEEN_ATTACK_OFFSETS;
      length = 8;
    }
    if (piece == Rook) {
      pieces = position->wrooks;
      offsets = ROOK_ATTACK_OFFSETS;
      length = 4;
    }
    if (piece == Bishop) {
      pieces = position->wbishops;
      offsets = BISHOP_ATTACK_OFFSETS;
      length = 4;
    }
  } else {
    friendly = black_piece_mask(position);
    if (piece == Queen) {
      pieces = position->bqueens;
      offsets = QUEEN_ATTACK_OFFSETS;
      length = 8;
    }
    if (piece == Rook) {
      pieces = position->brooks;
      offsets = ROOK_ATTACK_OFFSETS;
      length = 4;
    }
    if (piece == Bishop) {
      pieces = position->bbishops;
      offsets = BISHOP_ATTACK_OFFSETS;
      length = 4;
    }
  }

  while (pieces) {
    uint64_t p = pop_lsb(&pieces);
    uint64_t attack = slider_attack_mask(p, allOccupancy, offsets, length);
    while (attack) {
      uint64_t to = pop_lsb(&attack);
      if (to & friendly)
        continue;
      Piece toPiece = find_piece_type(to, position);
      Move m = {p, to, piece, toPiece, false, false, None};
      mArr->moves[mArr->count] = m;
      mArr->count++;
    }
  }
}

uint64_t slider_attack_mask(uint64_t piece, uint64_t allOccupancy, int offsets[], int length) {
  uint64_t attack = 0;
  for (int i = 0; i < length; i++) {
    attack |= ray_cast(piece, offsets[i], allOccupancy);
  }
  return attack;
}

AttackSegments slider_attack_mask_segmented(uint64_t *piece, uint64_t allOccupancy, int offsets[], int length) {
  AttackSegments sas;
  sas.length = length;
  for (int i = 0; i < length; i++) {
    sas.attacks[i] = ray_cast(*piece, offsets[i], allOccupancy);
  }
  return sas;
}
AttackSegments slider_reveal_check_mask_segmented(uint64_t *piece, int offsets[], int length, PositionContext context) {
  AttackSegments sas;
  sas.length = length;
  for (int i = 0; i < length; i++) {
    sas.attacks[i] = ray_cast_reveal_check(*piece, offsets[i], context);
  }
  return sas;
}
