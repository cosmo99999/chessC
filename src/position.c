#include "position.h"
#include "helpers.h"
#include "move.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

extern Zobrist *zobrist;
uint64_t get_random_uint64() {
  uint64_t x;
  getrandom(&x, sizeof(x), 0);
  return x;
}
Zobrist init_zobrist() {
  Zobrist z;
  uint64_t prev[1000];
  uint64_t prevCount = 0;
  for (int c = 0; c < 2; c++)
    for (int pt = 0; pt < 6; pt++)
      for (int sq = 0; sq < 64; sq++) {
        uint64_t num = get_random_uint64();
        prev[prevCount] = num;
        prevCount++;
        z.pieceKeys[c][pt][sq] = num;
      }

  for (int i = 0; i < 16; i++) {
    uint64_t num = get_random_uint64();
    prev[prevCount] = num;
    prevCount++;
    z.castlingKeys[i] = num;
  }

  for (int f = 0; f < 8; f++) {
    uint64_t num = get_random_uint64();
    prev[prevCount] = num;
    prevCount++;
    z.enPassantKeys[f] = num;
  }
  for (int i = 0; i < prevCount; i++) {
    for (int j = 0; j < prevCount; j++) {
      if (i == j)
        continue;
      if (prev[i] == prev[j]) {
        printf("Duplicate found in Zobrist gen \n");
        exit(1);
      }
    }
  }
  z.sideToMoveKey = get_random_uint64();
  return z;
}
uint64_t compute_zobrist(Position *position) {
  uint64_t hash = 0;

  for (int i = 0; i < 64; i++) {
    uint64_t pos = (1ULL << i);
    Piece type = find_piece_type(pos, position);
    Colour colour = find_piece_colour(pos, position);
    int cInt = -1;
    if (colour == Black)
      cInt = 0;
    else if (colour == White)
      cInt = 1;
    if (cInt != -1) {
      hash ^= zobrist->pieceKeys[cInt][(type)][i];
    }
  }
  if (!position->whitesMove) {
    hash ^= zobrist->sideToMoveKey;
  }
  hash ^= zobrist->castlingKeys[position->castlingRights];

  if (position->enPassantTile != 0)
    hash ^= zobrist->enPassantKeys[get_en_passant_file(position)];
  return hash;
}
Position start_position() {
  Position pos;
  pos.wpawns = get_row_mask(2);
  pos.wbishops = 0ULL | (1ULL << 2) | (1ULL << 5);
  pos.wknights = 0ULL | (1ULL << 1) | (1ULL << 6);
  pos.wrooks = 0ULL | (1ULL << 0) | (1ULL << 7);
  pos.wqueens = 0ULL | (1ULL << 3);
  pos.wking = 0ULL | (1ULL << 4);
  pos.bpawns = get_row_mask(7);
  pos.bbishops = 0ULL | (1ULL << 58) | (1ULL << 61);
  pos.bknights = 0ULL | (1ULL << 57) | (1ULL << 62);
  pos.brooks = 0ULL | (1ULL << 56) | (1ULL << 63);
  pos.bqueens = 0ULL | (1ULL << 59);
  pos.bking = 0ULL | (1ULL << 60);
  pos.fullMoveClock = 0;
  pos.halfMoveClock = 0;
  pos.whitesMove = true;
  pos.enPassantTile = 0;
  pos.castlingRights = 15;
  pos.zhash = compute_zobrist(&pos);
  return pos;
}
uint64_t white_piece_mask(Position *position) {
  return position->wpawns | position->wbishops | position->wrooks | position->wknights | position->wqueens |
         position->wking;
}
uint64_t black_piece_mask(Position *position) {
  return position->bpawns | position->bbishops | position->brooks | position->bknights | position->bqueens |
         position->bking;
}
UndoMove make_move(Move *move, Position *position) {
  UndoMove undo;
  undo.prevHash = position->zhash;
  undo.castlingRights = position->castlingRights;
  undo.enPassantTile = position->enPassantTile;
  undo.fullmoves = position->fullMoveClock;
  undo.halfmoves = position->halfMoveClock;
  if (move->castling) {
    Colour colour = position->whitesMove ? White : Black;
    uint64_t *rooks = position->whitesMove ? &position->wrooks : &position->brooks;
    uint64_t rookFrom = 0;
    uint64_t rookTo = 0;
    if (move->to == (1ULL << 6)) {
      rookFrom = (1ULL << 7);
      rookTo = (1ULL << 5);
    }
    if (move->to == (1ULL << 2)) {
      rookFrom = (1ULL << 0);
      rookTo = (1ULL << 3);
    }
    if (move->to == (1ULL << 62)) {
      rookFrom = (1ULL << 63);
      rookTo = (1ULL << 61);
    }
    if (move->to == (1ULL << 58)) {
      rookFrom = (1ULL << 56);
      rookTo = (1ULL << 59);
    }
    *rooks ^= rookFrom;
    *rooks |= rookTo;
    int toInt = lsb_get_int(&rookTo);
    int fromInt = lsb_get_int(&rookFrom);
    position->zhash ^= zobrist->pieceKeys[colour][Rook][fromInt];
    position->zhash ^= zobrist->pieceKeys[colour][Rook][toInt];
  }
  if (move->to == position->enPassantTile && move->pfrom == Pawn) {
    Colour colour;
    uint64_t pawnPos = 0;
    uint64_t *enemyPawns;
    int toInt = -1;
    if (position->whitesMove) {
      colour = Black;
      pawnPos = move->to >> 8;
      enemyPawns = find_piece_mask(pawnPos, position);
      toInt = lsb_get_int(&pawnPos);
    } else {
      colour = White;
      pawnPos = move->to << 8;
      enemyPawns = find_piece_mask(pawnPos, position);
      toInt = lsb_get_int(&pawnPos);
    }
    *enemyPawns ^= pawnPos;
    position->zhash ^= zobrist->pieceKeys[colour][Pawn][toInt];
  }
  if (move->promotion) {
    Colour colour = position->whitesMove ? White : Black;
    uint64_t *toPieces = find_piece_mask(move->to, position);
    int toInt = lsb_get_int(&move->to);
    int fromInt = lsb_get_int(&move->from);
    uint64_t *pawns = find_piece_mask(move->from, position);
    uint64_t *promotingPieces = find_piece_mask_by_type_and_colour(move->promotionPiece, colour, position);

    if (toPieces != NULL) {
      Colour takenPieceColour = position->whitesMove ? Black : White;
      Piece toPieceType = find_piece_type(move->to, position);
      *toPieces ^= move->to;
      position->zhash ^= zobrist->pieceKeys[takenPieceColour][toPieceType][toInt];
    }
    *pawns ^= move->from;
    position->zhash ^= zobrist->pieceKeys[colour][Pawn][fromInt];
    *promotingPieces |= move->to;
    position->zhash ^= zobrist->pieceKeys[colour][move->promotionPiece][toInt];
  } else {
    Colour fromColour = position->whitesMove ? White : Black;
    uint64_t *fromPieces = find_piece_mask(move->from, position);
    uint64_t *toPieces = find_piece_mask(move->to, position);
    int toInt = lsb_get_int(&move->to);
    int fromInt = lsb_get_int(&move->from);

    if (toPieces != NULL) {
      Colour toPieceColour = position->whitesMove ? Black : White;
      *toPieces ^= move->to;
      position->zhash ^= zobrist->pieceKeys[toPieceColour][move->pto][toInt];
    }
    *fromPieces ^= move->from;
    *fromPieces |= move->to;
    position->zhash ^= zobrist->pieceKeys[fromColour][move->pfrom][fromInt];
    position->zhash ^= zobrist->pieceKeys[fromColour][move->pfrom][toInt];
  }
  // castle rights check
  position->zhash ^= zobrist->castlingKeys[position->castlingRights];
  if (move->pfrom == King || move->pfrom == Rook || move->pto == Rook) {
    uint64_t wks = 1ULL;
    uint64_t wqs = 2ULL;
    uint64_t bks = 4ULL;
    uint64_t bqs = 8ULL;
    if (move->from == 1ULL << 4) {
      if (position->castlingRights & wks)
        position->castlingRights ^= wks;
      if (position->castlingRights & wqs)
        position->castlingRights ^= wqs;
    }
    if (move->from == 1ULL << 60) {
      if (position->castlingRights & bks)
        position->castlingRights ^= bks;
      if (position->castlingRights & bqs)
        position->castlingRights ^= bqs;
    }
    if (move->from == 1ULL || move->to == 1ULL) {
      if (position->castlingRights & wqs)
        position->castlingRights ^= wqs;
    }
    if (move->from == 1ULL << 7 || move->to == 1ULL << 7) {
      if (position->castlingRights & wks)
        position->castlingRights ^= wks;
    }
    if (move->from == 1ULL << 56 || move->to == 1ULL << 56) {
      if (position->castlingRights & bqs)
        position->castlingRights ^= bqs;
    }
    if (move->from == 1ULL << 63 || move->to == 1ULL << 63) {
      if (position->castlingRights & bks)
        position->castlingRights ^= bks;
    }
  }
  position->zhash ^= zobrist->castlingKeys[position->castlingRights];
  // en passant check
  if (position->enPassantTile != 0) {
    position->zhash ^= zobrist->enPassantKeys[get_en_passant_file(position)];
    position->enPassantTile = 0;
  }
  if (move->pfrom == Pawn && (move->from & ROW_TWO || move->from & ROW_SEVEN)) {
    uint64_t enPassantTile = 0;
    if (move->to & ROW_FOUR) {
      enPassantTile = move->to >> 8;
    } else if (move->to & ROW_FIVE) {
      enPassantTile = move->to << 8;
    }
    if (enPassantTile != 0) {
      position->enPassantTile = enPassantTile;
      position->zhash ^= zobrist->enPassantKeys[get_en_passant_file(position)];
    }
  }
  position->halfMoveClock++;
  if (move->pto != None || move->pfrom == Pawn) {
    position->halfMoveClock = 0;
  }
  position->fullMoveClock++;
  position->zhash ^= zobrist->sideToMoveKey;
  position->whitesMove = !position->whitesMove;
  return undo;
};
void unmake_move(Move *move, Position *position, UndoMove *undo) {
  if (move->castling) {
    uint64_t *rooks = position->whitesMove ? &position->brooks : &position->wrooks;
    uint64_t rookFrom = 0;
    uint64_t rookTo = 0;
    if (move->to == (1ULL << 6)) {
      rookFrom = (1ULL << 7);
      rookTo = (1ULL << 5);
    }
    if (move->to == (1ULL << 2)) {
      rookFrom = (1ULL << 0);
      rookTo = (1ULL << 3);
    }
    if (move->to == (1ULL << 62)) {
      rookFrom = (1ULL << 63);
      rookTo = (1ULL << 61);
    }
    if (move->to == (1ULL << 58)) {
      rookFrom = (1ULL << 56);
      rookTo = (1ULL << 59);
    }
    *rooks |= rookFrom;
    *rooks ^= rookTo;
  }
  if (move->to == undo->enPassantTile && move->pfrom == Pawn) {
    uint64_t pawnPos = 0;
    uint64_t *enemyPawns;
    if (position->whitesMove) {
      pawnPos = move->to << 8;
      enemyPawns = find_piece_mask_by_type_and_colour(Pawn, White, position);
    } else {
      pawnPos = move->to >> 8;
      enemyPawns = find_piece_mask_by_type_and_colour(Pawn, Black, position);
    }
    *enemyPawns |= pawnPos;
  }
  if (move->promotion) {
    Colour movingColour = position->whitesMove ? Black : White;
    Colour enemyColour = position->whitesMove ? White : Black;
    uint64_t *promotedPiece = find_piece_mask(move->to, position);
    uint64_t *pawns = find_piece_mask_by_type_and_colour(Pawn, movingColour, position);
    uint64_t *capturedPiece = find_piece_mask_by_type_and_colour(move->pto, enemyColour, position);

    if (capturedPiece != NULL) {
      *capturedPiece |= move->to;
    }
    *pawns |= move->from;
    *promotedPiece ^= move->to;
  } else {
    Colour toColour = position->whitesMove ? White : Black;
    uint64_t *movedPiece = find_piece_mask(move->to, position);
    uint64_t *capturedPiece = find_piece_mask_by_type_and_colour(move->pto, toColour, position);
    if (capturedPiece != NULL) {
      *capturedPiece |= move->to;
    }

    *movedPiece |= move->from;
    *movedPiece ^= move->to;
  }
  position->castlingRights = undo->castlingRights;
  position->enPassantTile = undo->enPassantTile;
  position->halfMoveClock = undo->halfmoves;
  position->fullMoveClock = undo->fullmoves;
  position->whitesMove = !position->whitesMove;
}
