#include "helpers.h"
#include "move.h"
#include <stdio.h>
#include <string.h>

typedef struct {
  uint64_t from;
  uint64_t to;
  Piece promotionPiece;
} ParsedMove;

static inline char **split_by_whitespace(const char *cstr, int *word_count) {
  // Duplicate the string because strtok modifies the input string
  char *str_copy = strdup(cstr);
  if (!str_copy)
    return NULL;

  int capacity = 10;
  int count = 0;
  char **words = (char **)malloc(capacity * sizeof(char *));
  if (!words) {
    free(str_copy);
    return NULL;
  }

  // Define whitespace characters: space, tab, newline
  const char *delimiters = " \t\r\n";
  char *token = strtok(str_copy, delimiters);

  while (token != NULL) {
    // Resize array if it runs out of capacity
    if (count >= capacity) {
      capacity *= 2;
      words = (char **)realloc(words, capacity * sizeof(char *));
    }

    // Duplicate the token to store it in the array
    words[count++] = strdup(token);
    token = strtok(NULL, delimiters);
  }

  free(str_copy); // Free the temporary copy
  *word_count = count;
  return words;
}
static inline void free_word_array(char **words, int count) {
  for (int i = 0; i < count; i++) {
    free(words[i]);
  }
  free(words);
}

static inline void square_to_str(int square, char *buf) {
  buf[0] = 'a' + (square % 8); // file
  buf[1] = '1' + (square / 8); // rank
}
static inline int str_to_square(const char *buf) {
  int file = buf[0] - 'a'; // 'a' -> 0, 'h' -> 7
  int rank = buf[1] - '1'; // '1' -> 0, '8' -> 7
  return rank * 8 + file;
}
static inline uint64_t str_to_bit(const char *buf) {
  int square = str_to_square(buf);
  return 1ULL << square;
}
static inline void print_move_uci(uint64_t from, uint64_t to, Piece promotionPiece) {
  int fromSq = lsb_get_int(&from);
  int toSq = lsb_get_int(&to);

  char buf[6] = {0}; // "e7e8q\0" worst case

  square_to_str(toSq, &buf[2]);
  square_to_str(fromSq, &buf[0]);

  if (promotionPiece != None) {
    char promoChar;
    switch (promotionPiece) {
    case Queen:
      promoChar = 'q';
      break;
    case Rook:
      promoChar = 'r';
      break;
    case Bishop:
      promoChar = 'b';
      break;
    case Knight:
      promoChar = 'n';
      break;
    default:
      promoChar = '?';
      break; // shouldn't happen
    }
    buf[4] = promoChar;
    buf[5] = '\0';
  } else {
    buf[4] = '\0';
  }

  printf("%s", buf);
}
static inline void read_input(char *buf, size_t bufSize) {
  if (fgets(buf, bufSize, stdin) == NULL) {
    buf[0] = '\0';
    return;
  }
  size_t len = strlen(buf);
  if (len > 0 && buf[len - 1] == '\n') {
    buf[len - 1] = '\0';
  }
  if (len > 1 && buf[len - 2] == '\r') { // handle Windows-style line endings
    buf[len - 2] = '\0';
  }
}
static inline bool equals(char *input, char *value) {
  if (strcmp(input, value) == 0) {
    return true;
  } else {
    return false;
  }
}
static inline ParsedMove parse_uci_move(const char *str) {
  ParsedMove pm;
  pm.from = str_to_bit(&str[0]);
  pm.to = str_to_bit(&str[2]);
  pm.promotionPiece = None;

  if (str[4] != '\0') {
    switch (str[4]) {
    case 'q':
      pm.promotionPiece = Queen;
      break;
    case 'r':
      pm.promotionPiece = Rook;
      break;
    case 'b':
      pm.promotionPiece = Bishop;
      break;
    case 'n':
      pm.promotionPiece = Knight;
      break;
    }
  }

  return pm;
}
static inline Position position_from_fen(const char *fen) {
  Position result;
  memset(&result, 0, sizeof(Position));

  char boardString[100] = {0};
  char turn[8] = {0};
  char castling[10] = {0};
  char passant[10] = {0};
  char halfmoves[16] = "0";
  char fullmoves[16] = "1";

  // sscanf handles the whitespace-splitting that getline(..., ' ') did
  sscanf(fen, "%99s %7s %9s %9s %15s %15s", boardString, turn, castling, passant, halfmoves, fullmoves);

  int x = 0;
  int y = 7;
  for (int i = 0; boardString[i] != '\0'; i++) {
    char c = boardString[i];
    int pos = x + y * 8;

    if (c == '/') {
      x = 0;
      --y;
    } else if (c >= '0' && c <= '9') {
      x += c - '0';
    } else {
      switch (c) {
      case 'P':
        result.wpawns |= 1ULL << pos;
        ++x;
        break;
      case 'N':
        result.wknights |= 1ULL << pos;
        ++x;
        break;
      case 'B':
        result.wbishops |= 1ULL << pos;
        ++x;
        break;
      case 'R':
        result.wrooks |= 1ULL << pos;
        ++x;
        break;
      case 'Q':
        result.wqueens |= 1ULL << pos;
        ++x;
        break;
      case 'K':
        result.wking |= 1ULL << pos;
        ++x;
        break;
      case 'p':
        result.bpawns |= 1ULL << pos;
        ++x;
        break;
      case 'n':
        result.bknights |= 1ULL << pos;
        ++x;
        break;
      case 'b':
        result.bbishops |= 1ULL << pos;
        ++x;
        break;
      case 'r':
        result.brooks |= 1ULL << pos;
        ++x;
        break;
      case 'q':
        result.bqueens |= 1ULL << pos;
        ++x;
        break;
      case 'k':
        result.bking |= 1ULL << pos;
        ++x;
        break;
      default:
        break;
      }
    }
  }

  result.whitesMove = (turn[0] != 'b');

  result.castlingRights = 0;
  for (int i = 0; castling[i] != '\0'; i++) {
    switch (castling[i]) {
    case 'K':
      result.castlingRights |= 1;
      break;
    case 'Q':
      result.castlingRights |= 2;
      break;
    case 'k':
      result.castlingRights |= 4;
      break;
    case 'q':
      result.castlingRights |= 8;
      break;
    default:
      break;
    }
  }

  result.enPassantTile = 0;
  if (passant[0] != '-' && passant[0] != '\0' && passant[1] != '\0') {
    int passant_x = passant[0] - 'a';
    int passant_y = passant[1] - '1';
    int passantTile = passant_x + passant_y * 8;
    result.enPassantTile = 1ULL << passantTile;
  }

  result.halfMoveClock = atoi(halfmoves);
  result.fullMoveClock = atoi(fullmoves);

  return result;
}
