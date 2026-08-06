#include "position.h"
#include <stdint.h>

uint64_t white_piece_mask(Position *position) {
  return position->wpawns | position->wbishops | position->wrooks | position->wknights | position->wqueens |
         position->wking;
}

uint64_t black_piece_mask(Position *position) {
  return position->bpawns | position->bbishops | position->brooks | position->bknights | position->bqueens |
         position->bking;
}

uint64_t *find_piece_mask(uint64_t mask, Position *position) {
  if (mask & position->wpawns)
    return &position->wpawns;
  if (mask & position->bpawns)
    return &position->bpawns;
  if (mask & position->wknights)
    return &position->wknights;
  if (mask & position->wbishops)
    return &position->wbishops;
  if (mask & position->bbishops)
    return &position->bbishops;
  if (mask & position->brooks)
    return &position->brooks;
  if (mask & position->wrooks)
    return &position->wrooks;
  if (mask & position->wqueens)
    return &position->wqueens;
  if (mask & position->bqueens)
    return &position->bqueens;
  if (mask & position->wking)
    return &position->wking;
  if (mask & position->bking)
    return &position->bking;
  return NULL;
}
Piece find_piece_type(uint64_t mask, Position *position) {
  if (mask & (position->wpawns | position->bpawns))
    return Pawn;
  if (mask & (position->wknights | position->bknights))
    return Knight;
  if (mask & (position->bbishops | position->wbishops))
    return Bishop;
  if (mask & (position->brooks | position->wrooks))
    return Rook;
  if (mask & (position->bqueens | position->wqueens))
    return Queen;
  if (mask & (position->bking | position->wking))
    return King;
  return None;
}
