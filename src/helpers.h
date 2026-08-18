#pragma once
#include "move.h"
#include "position.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#ifndef CHESS_HELPERS
#define CHESS_HELPERS

static inline void print_binary_grid(uint64_t value) {
  for (int i = 7; i > -1; i--) {
    for (int j = 0; j < 8; j++) {
      if (value & (1ULL << ((i * 8) + j))) {
        printf("1");
      } else {
        printf("0");
      }
    }
    printf("\n");
  }
}
static inline uint64_t pop_lsb(uint64_t *bit) {
  int pos = __builtin_ctzll(*bit);
  uint64_t bitPos = (1ULL << pos);
  *bit ^= bitPos;
  return bitPos;
}
static inline int pop_lsb_get_int(uint64_t *bit) {
  int pos = __builtin_ctzll(*bit);
  uint64_t bitPos = (1ULL << pos);
  *bit ^= bitPos;
  return pos;
}
static inline int lsb_get_int(uint64_t *bit) {
  int pos = __builtin_ctzll(*bit);
  return pos;
}
static inline int max(int a, int b) {
  if (a > b)
    return a;
  return b;
}
static inline int min(int a, int b) {
  if (a < b)
    return a;
  return b;
}

static inline uint64_t get_boundary(int offset) {
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
    return ROW_ONE | H_FILE;
  }
  if (offset == 9) {
    return ROW_EIGHT | H_FILE;
  }
  if (offset == -9) {
    return ROW_ONE | A_FILE;
  }
  return 0;
}
static inline uint64_t ray_cast_reveal_check(uint64_t piece, int offset, PositionContext context) {
  uint64_t attack = 0;
  uint64_t nextPos = piece;
  int absOffset = abs(offset);
  uint64_t boundary = get_boundary(offset);
  bool oneDefender = false;
  // slide in direction until it hits one defending piece and then the king without other pieces in the way
  while (!(nextPos & boundary)) {
    if (offset > 0) {
      nextPos = nextPos << absOffset;
    } else {
      nextPos = nextPos >> absOffset;
    }
    attack |= nextPos;

    if ((nextPos & context.king) && oneDefender) {
      return attack;
    }
    if (nextPos & context.attackingPieces) {
      return 0;
    }
    if (nextPos & context.friendlyPieces) {
      if (oneDefender) {
        return 0;
      }
      oneDefender = true;
    }
  }
  return 0;
}
static inline uint64_t ray_cast(uint64_t piece, int offset, uint64_t occupancy) {
  uint64_t attack = 0;
  uint64_t nextPos = piece;
  int absOffset = abs(offset);
  uint64_t boundary = get_boundary(offset);

  while (!(nextPos & boundary)) {
    if (offset > 0) {
      nextPos = nextPos << absOffset;
    } else {
      nextPos = nextPos >> absOffset;
    }
    attack |= nextPos;
    if (nextPos & occupancy) {
      break;
    }
  }
  return attack;
}
static inline void get_piece_name(char *buffer, size_t max_size, Piece piece) {
  if (piece == Pawn) {
    strncpy(buffer, "Pawn", max_size - 1);
    buffer[max_size - 1] = '0';
    return;
  }
  if (piece == Knight) {
    strncpy(buffer, "Night", max_size - 1);
    buffer[max_size - 1] = '0';
    return;
  }
  if (piece == Bishop) {
    strncpy(buffer, "Bishop", max_size - 1);
    buffer[max_size - 1] = '0';
    return;
  }
  if (piece == Queen) {
    strncpy(buffer, "Queen", max_size - 1);
    buffer[max_size - 1] = '0';
    return;
  }
  if (piece == Rook) {
    strncpy(buffer, "Rook", max_size - 1);
    buffer[max_size - 1] = '0';
    return;
  }
  if (piece == King) {
    strncpy(buffer, "King", max_size - 1);
    buffer[max_size - 1] = '0';
    return;
  }

  strncpy(buffer, "None", max_size - 1);
  buffer[max_size - 1] = '0';
}
static inline void print_move(Move *m) {
  char fromName[10];
  char toName[10];
  char promotionName[10];
  get_piece_name(fromName, sizeof(fromName), m->pfrom);
  get_piece_name(toName, sizeof(toName), m->pto);
  get_piece_name(promotionName, sizeof(promotionName), m->promotionPiece);
  printf("Piece: %s \n", fromName);
  printf("from: %d \n", lsb_get_int(&m->from));
  printf("to: %d \n", lsb_get_int(&m->to));
  printf("taking: %s \n", toName);
  if (m->castling) {
    printf("castling!");
  }
  if (m->promotion) {
    printf("promotion!");
    printf("promoting to: %s \n", promotionName);
  }
  printf("\n");
}
static inline uint64_t get_file_mask(int f) {
  uint64_t file = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (j == f - 1) {
        file |= 1ULL << (i * 8 + j);
      }
    }
  }
  return file;
}
static inline uint64_t get_row_mask(int row) {
  uint64_t file = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (i == row - 1) {
        file |= 1ULL << (i * 8 + j);
      }
    }
  }
  return file;
}
static inline int get_mask_with_length(int length) {
  int result = 0;
  for (int i = 0; i < length; i++) {
    result |= 1 << i;
  };
  return result;
}
static inline int value_from(int value, int start, int length) {
  int shifted = value >> start;
  int mask = get_mask_with_length(length);
  return shifted & (mask);
}
static inline uint64_t *find_piece_mask(uint64_t mask, Position *position) {
  if (mask & position->wpawns)
    return &position->wpawns;
  if (mask & position->bpawns)
    return &position->bpawns;
  if (mask & position->wknights)
    return &position->wknights;
  if (mask & position->bknights)
    return &position->bknights;
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
static inline uint64_t *find_piece_mask_by_type_and_colour(Piece piece, Colour colour, Position *position) {
  if (colour == White) {
    if (piece == Pawn) {
      return &position->wpawns;
    }
    if (piece == Bishop) {
      return &position->wbishops;
    }
    if (piece == Rook) {
      return &position->wrooks;
    }
    if (piece == Queen) {
      return &position->wqueens;
    }
    if (piece == Knight) {
      return &position->wknights;
    }
    if (piece == King) {
      return &position->wking;
    }
  } else {
    if (piece == Pawn) {
      return &position->bpawns;
    }
    if (piece == Bishop) {
      return &position->bbishops;
    }
    if (piece == Rook) {
      return &position->brooks;
    }
    if (piece == Queen) {
      return &position->bqueens;
    }
    if (piece == Knight) {
      return &position->bknights;
    }
    if (piece == King) {
      return &position->bking;
    }
  }
  return NULL;
}
static inline Piece find_piece_type(uint64_t mask, Position *position) {
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
static inline Colour find_piece_colour(uint64_t mask, Position *position) {
  if (white_piece_mask(position) & mask) {
    return White;
  }
  if (black_piece_mask(position) & mask) {
    return Black;
  }
  return NoColour;
}
static inline int get_en_passant_file(Position *position) {
  if (position->enPassantTile & A_FILE) {
    return 0;
  }
  if (position->enPassantTile & B_FILE) {
    return 1;
  }
  if (position->enPassantTile & C_FILE) {
    return 2;
  }
  if (position->enPassantTile & D_FILE) {
    return 3;
  }
  if (position->enPassantTile & E_FILE) {
    return 4;
  }
  if (position->enPassantTile & F_FILE) {
    return 5;
  }
  if (position->enPassantTile & G_FILE) {
    return 6;
  }
  if (position->enPassantTile & H_FILE) {
    return 7;
  }
  return 0;
}
static inline void print_board(Position *position) {
  for (int i = 7; i > -1; i--) {
    for (int j = 0; j < 8; j++) {
      uint64_t tile = (1ULL << ((i * 8) + j));
      Piece piece = find_piece_type(tile, position);
      char pname[50];
      get_piece_name(pname, sizeof(pname), piece);
      if (piece == None) {
        printf("-");
        continue;
      }
      if (tile & white_piece_mask(position)) {
        char out = toupper(pname[0]);
        printf("%c", out);
      } else {
        char out = tolower(pname[0]);
        printf("%c", out);
      }
    }
    printf("\n");
  }
}
static inline uint64_t king_row(Position *position) {
  uint64_t king = position->whitesMove ? position->wking : position->bking;
  if (king & ROW_ONE)
    return ROW_ONE;
  if (king & ROW_TWO)
    return ROW_TWO;
  if (king & ROW_THREE)
    return ROW_THREE;
  if (king & ROW_FOUR)
    return ROW_FOUR;
  if (king & ROW_FIVE)
    return ROW_FIVE;
  if (king & ROW_SIX)
    return ROW_SIX;
  if (king & ROW_SEVEN)
    return ROW_SEVEN;
  if (king & ROW_EIGHT)
    return ROW_EIGHT;
  return 0;
}
static inline uint64_t king_file(Position *position) {
  uint64_t king = position->whitesMove ? position->wking : position->bking;
  if (king & A_FILE)
    return A_FILE;
  if (king & B_FILE)
    return B_FILE;
  if (king & C_FILE)
    return C_FILE;
  if (king & D_FILE)
    return D_FILE;
  if (king & E_FILE)
    return E_FILE;
  if (king & F_FILE)
    return F_FILE;
  if (king & G_FILE)
    return G_FILE;
  if (king & H_FILE)
    return H_FILE;
  return 0;
}
static inline uint64_t sum_ULL_arr(uint64_t *arr, int length) {
  uint64_t result = 0;
  for (int i = 0; i < length; i++) {
    result |= arr[i];
  }
  return result;
}
#endif
