#pragma once
#include "move.h"
#include <stdint.h>
#include <stdlib.h>

inline uint64_t pop_lsb(uint64_t *bit) {
  int pos = __builtin_ctzll(*bit);
  uint64_t bitPos = 1 << (0ULL + pos);
  *bit ^= bitPos;
  return bitPos;
}
inline uint64_t get_boundary(int offset) {
  if (offset == -1) {
    return A_FILE;
  }
  if (offset == 1) {
    return H_FILE;
  }
  if (offset == 8) {
    return ROW_EIGHT;
  }
  if (offset == -8) {
    return ROW_ONE;
  }
  if (offset == 7) {
    return ROW_EIGHT | A_FILE;
  }
  if (offset == -7) {
    return ROW_ONE | A_FILE;
  }
  if (offset == 9) {
    return ROW_EIGHT | H_FILE;
  }
  if (offset == -9) {
    return ROW_ONE | A_FILE;
  }
  return 0;
}
inline uint64_t ray_cast(uint64_t piece, int offset) {
  uint64_t attack = 0;
  uint64_t nextPos = 0;
  int absOffset = abs(offset);
  uint64_t boundary = get_boundary(offset);

  while (!(nextPos & boundary)) {
    if (offset > 0) {
      nextPos << absOffset;
    } else {
      nextPos >> absOffset;
    }
  }
}
