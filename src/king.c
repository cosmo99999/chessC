#include "helpers.h"
#include "move.h"
#include "position.h"
#include <stdint.h>

#ifndef CASTLE_MASKS
#define CASTLE_MASKS
#define WHITE_QUEEN_SIDE 14ULL
#define WHITE_QUEEN_SIDE_MOVING 12ULL
#define BLACK_QUEEN_SIDE 1008806316530991104ULL
#define BLACK_QUEEN_SIDE_MOVING 864691128455135232ULL
#define WHITE_KING_SIDE 96ULL
#define BLACK_KING_SIDE 6917529027641081856ULL
#endif

void king_moves(MoveArr *mArr, Position *position, AttackerInfo *aInfo) {
  uint64_t king = 0;
  uint64_t allOccupancy = white_piece_mask(position) | black_piece_mask(position);
  uint64_t friendly = 0;

  // castling moves
  if (position->whitesMove) {
    king = position->wking;
    friendly = white_piece_mask(position);

    if (position->castlingRights & (1ULL)) {
      if (!(allOccupancy & WHITE_KING_SIDE)) {
        if (!(aInfo->fullAttackMask & WHITE_KING_SIDE) && !aInfo->check) {
          Move m = {king, (1ULL << 6), King, None, true, false, None};
          mArr->moves[mArr->count] = m;
          mArr->count++;
        }
      }
    }
    if (position->castlingRights & (2ULL)) {
      if (!(allOccupancy & WHITE_QUEEN_SIDE)) {
        if (!(aInfo->fullAttackMask & WHITE_QUEEN_SIDE_MOVING) && !aInfo->check) {
          Move m = {king, (1ULL << 2), King, None, true, false, None};
          mArr->moves[mArr->count] = m;
          mArr->count++;
        }
      }
    }
  } else {
    king = position->bking;
    friendly = black_piece_mask(position);

    if (position->castlingRights & (4ULL)) {
      if (!(allOccupancy & BLACK_KING_SIDE)) {
        if (!(aInfo->fullAttackMask & BLACK_KING_SIDE) && !aInfo->check) {
          Move m = {king, (1ULL << 62), King, None, true, false, None};
          mArr->moves[mArr->count] = m;
          mArr->count++;
        }
      }
    }
    if (position->castlingRights & (8ULL)) {
      if (!(allOccupancy & BLACK_QUEEN_SIDE)) {
        if (!(aInfo->fullAttackMask & BLACK_QUEEN_SIDE_MOVING) && !aInfo->check) {
          Move m = {king, (1ULL << 58), King, None, true, false, None};
          mArr->moves[mArr->count] = m;
          mArr->count++;
        }
      }
    }
  }

  uint64_t attack = king_attack_mask(&king);
  while (attack) {
    uint64_t to = pop_lsb(&attack);
    if (to & friendly)
      continue;
    Piece toPiece = find_piece_type(to, position);
    Move m = {king, to, King, toPiece, false, false, None};
    mArr->moves[mArr->count] = m;
    mArr->count++;
  }
}

uint64_t king_attack_mask(uint64_t *king) {
  uint64_t k = *king;
  uint64_t attack = 0;
  if (!(k & A_FILE)) {
    attack |= k >> 1;
  }
  if (!(k & H_FILE)) {
    attack |= k << 1;
  }
  if (!(k & ROW_EIGHT)) {
    attack |= k << 8;
  }
  if (!(k & ROW_ONE)) {
    attack |= k >> 8;
  }
  if (!(k & (ROW_ONE | A_FILE))) {
    attack |= k >> 9;
  }
  if (!(k & (ROW_ONE | H_FILE))) {
    attack |= k >> 7;
  }
  if (!(k & (ROW_EIGHT | H_FILE))) {
    attack |= k << 9;
  }
  if (!(k & (ROW_EIGHT | A_FILE))) {
    attack |= k << 7;
  }
  return attack;
}
