#include "move.h"
#include "helpers.h"
#include "position.h"
#include <stdint.h>

MoveArr get_moves(Position *position) {

  MoveArr mArr;
  mArr.count = 0;

  AttackerInfo aInfo = get_attacker_info(position);
  RevealAttackSegments revealAttacks = get_reveal_check_lines(position);

  pawn_moves(&mArr, position);
  knight_moves(&mArr, position);
  slider_moves(&mArr, position, Bishop);
  slider_moves(&mArr, position, Queen);
  slider_moves(&mArr, position, Rook);
  king_moves(&mArr, position, &aInfo);

  // filter king moving into check
  for (int i = mArr.count - 1; i > -1; i--) {
    Move *m = &mArr.moves[i];
    if (m->pfrom == King) {
      if (m->to & aInfo.fullAttackMask) {
        mArr.moves[i] = mArr.moves[mArr.count - 1];
        mArr.count--;
      }
    }
  }
  // filter all moves but king moves if in multicheck
  if (aInfo.multicheck) {
    for (int i = mArr.count - 1; i > -1; i--) {
      Move *m = &mArr.moves[i];
      if (m->pfrom != King) {
        mArr.moves[i] = mArr.moves[mArr.count - 1];
        mArr.count--;
      }
    }
  } else if (aInfo.check) {
    // if in check with a slider threat
    if (aInfo.checkMask) {
      for (int i = mArr.count - 1; i > -1; i--) {
        Move *m = &mArr.moves[i];
        if (m->pfrom == King) {
          if (!(m->to & aInfo.checkMask) && !(m->to & aInfo.checkingPiecePos)) {
            uint64_t *k = find_piece_mask(m->from, position);
            uint64_t king = *k;

            king ^= m->from;
            king |= m->to;
            Piece type = find_piece_type(aInfo.checkingPiecePos, position);
            uint64_t allPieces = black_piece_mask(position) | white_piece_mask(position);
            allPieces ^= position->whitesMove ? position->wking : position->bking;
            allPieces |= king;
            uint64_t newAttackMask = 0;
            if (type == Bishop) {
              newAttackMask = slider_attack_mask(&aInfo.checkingPiecePos, allPieces, BISHOP_ATTACK_OFFSETS, 4);
            } else if (type == Queen) {
              newAttackMask = slider_attack_mask(&aInfo.checkingPiecePos, allPieces, QUEEN_ATTACK_OFFSETS, 8);
            } else if (type == Rook) {
              newAttackMask = slider_attack_mask(&aInfo.checkingPiecePos, allPieces, ROOK_ATTACK_OFFSETS, 4);
            }
            if (newAttackMask & king) {
              mArr.moves[i] = mArr.moves[mArr.count - 1];
              mArr.count--;
            }
            king ^= m->to;
            king |= m->from;
          }
        } else if (!((m->to & aInfo.checkMask) || (m->to & aInfo.checkingPiecePos))) {
          mArr.moves[i] = mArr.moves[mArr.count - 1];
          mArr.count--;
        }
      }
    }
    // if in check without a slider threat, ie knight or pawn
    else {
      for (int i = mArr.count - 1; i > -1; i--) {
        Move *m = &mArr.moves[i];
        if (m->pfrom == King)
          continue;
        if (m->to != aInfo.checkingPiecePos) {
          mArr.moves[i] = mArr.moves[mArr.count - 1];
          mArr.count--;
        }
      }
    }
  }

  for (int i = 0; i < revealAttacks.length; i++) {
    uint64_t attack = revealAttacks.attacks[i].attack;
    uint64_t attackerPos = revealAttacks.attacks[i].piecePos;
    for (int i = mArr.count - 1; i > -1; i--) {
      Move *m = &mArr.moves[i];
      if (m->pfrom == King) {
        continue;
      }
      if (m->from & attack) {
        if (!((m->to & attack) || (m->to & attackerPos))) {
          mArr.moves[i] = mArr.moves[mArr.count - 1];
          mArr.count--;
          if (mArr.count == 0) {
            return mArr;
          }
        }
      }
    }
  }

  return mArr;
}
AttackerInfo get_attacker_info(Position *position) {
  uint64_t pawns = 0;
  uint64_t knights = 0;
  uint64_t bishops = 0;
  uint64_t rooks = 0;
  uint64_t queens = 0;
  uint64_t king = 0;
  uint64_t myKing = 0;
  uint64_t allOccupancy = black_piece_mask(position) | white_piece_mask(position);
  AttackerInfo aInfo;
  aInfo.checkingPiecePos = 0;
  aInfo.fullAttackMask = 0;
  aInfo.multicheck = false;
  aInfo.check = false;
  aInfo.checkMask = 0;

  if (position->whitesMove) {
    pawns = position->bpawns;
    knights = position->bknights;
    bishops = position->bbishops;
    rooks = position->brooks;
    queens = position->bqueens;
    king = position->bking;
    myKing = position->wking;
    while (pawns) {
      uint64_t p = pop_lsb(&pawns);
      uint64_t attack = bpawn_attack_mask(&p);
      aInfo.fullAttackMask |= attack;
      if (attack & myKing) {
        aInfo.checkingPiecePos = p;
        aInfo.check = true;
      }
    }
  } else {
    pawns = position->wpawns;
    knights = position->wknights;
    bishops = position->wbishops;
    rooks = position->wrooks;
    queens = position->wqueens;
    king = position->wking;
    myKing = position->bking;
    while (pawns) {
      uint64_t p = pop_lsb(&pawns);
      uint64_t attack = wpawn_attack_mask(&p);
      aInfo.fullAttackMask |= attack;
      if (attack & myKing) {
        aInfo.checkingPiecePos = p;
        aInfo.check = true;
      }
    }
  }

  while (knights) {
    uint64_t k = pop_lsb(&knights);
    uint64_t attack = knight_attack_mask(&k);
    aInfo.fullAttackMask |= attack;
    if (attack & myKing) {
      aInfo.checkingPiecePos = k;
      aInfo.check = true;
    }
  }
  /*
   * SAS gets all the individual attacks per direction, to then return to use to restrict piece movements
   * if in check for that exact part of the slider attack that is creating check
   */
  while (bishops) {
    uint64_t b = pop_lsb(&bishops);
    AttackSegments bas = slider_attack_mask_segmented(&b, allOccupancy, BISHOP_ATTACK_OFFSETS, 4);
    aInfo.fullAttackMask |= sum_ULL_arr(bas.attacks, bas.length);

    for (int i = 0; i < bas.length; i++) {
      if (bas.attacks[i] & myKing) {
        if (aInfo.check)
          aInfo.multicheck = true;
        aInfo.checkMask = bas.attacks[i];
        aInfo.checkingPiecePos = b;
        aInfo.check = true;
      }
    }
  }
  while (rooks) {
    uint64_t r = pop_lsb(&rooks);
    AttackSegments ras = slider_attack_mask_segmented(&r, allOccupancy, ROOK_ATTACK_OFFSETS, 4);
    aInfo.fullAttackMask |= sum_ULL_arr(ras.attacks, ras.length);

    for (int i = 0; i < ras.length; i++) {
      if (ras.attacks[i] & myKing) {
        if (aInfo.check)
          aInfo.multicheck = true;
        aInfo.checkMask = ras.attacks[i];
        aInfo.checkingPiecePos = r;
        aInfo.check = true;
      }
    }
  }
  while (queens) {
    uint64_t q = pop_lsb(&queens);
    AttackSegments qas = slider_attack_mask_segmented(&q, allOccupancy, QUEEN_ATTACK_OFFSETS, 8);
    aInfo.fullAttackMask |= sum_ULL_arr(qas.attacks, qas.length);

    for (int i = 0; i < qas.length; i++) {
      if (qas.attacks[i] & myKing) {
        if (aInfo.check)
          aInfo.multicheck = true;
        aInfo.check = true;
        aInfo.checkMask = qas.attacks[i];
        aInfo.checkingPiecePos = q;
      }
    }
  }
  aInfo.fullAttackMask |= king_attack_mask(&king);

  return aInfo;
}
RevealAttackSegments get_reveal_check_lines(Position *position) {
  RevealAttackSegments result;
  result.length = 0;
  uint64_t king = 0;
  uint64_t bishops = 0;
  uint64_t rooks = 0;
  uint64_t queens = 0;
  PositionContext context;

  if (position->whitesMove) {
    king = position->wking;
    bishops = position->bbishops;
    rooks = position->brooks;
    queens = position->bqueens;
    context.king = king;
    context.attackingPieces = black_piece_mask(position);
    context.friendlyPieces = white_piece_mask(position);
  } else {
    king = position->bking;
    bishops = position->wbishops;
    rooks = position->wrooks;
    queens = position->wqueens;
    context.king = king;
    context.attackingPieces = white_piece_mask(position);
    context.friendlyPieces = black_piece_mask(position);
  }

  while (bishops) {
    uint64_t b = pop_lsb(&bishops);
    AttackSegments bas = slider_reveal_check_mask_segmented(&b, BISHOP_ATTACK_OFFSETS, 4, context);
    for (int i = 0; i < 4; i++) {
      if (bas.attacks[i] != 0) {
        result.attacks[result.length].attack = bas.attacks[i];
        result.attacks[result.length].piecePos = b;
        result.length++;
      }
    }
  }
  while (queens) {
    uint64_t q = pop_lsb(&queens);
    AttackSegments qas = slider_reveal_check_mask_segmented(&q, QUEEN_ATTACK_OFFSETS, 8, context);
    for (int i = 0; i < 8; i++) {
      if (qas.attacks[i] != 0) {
        result.attacks[result.length].attack = qas.attacks[i];
        result.attacks[result.length].piecePos = q;
        result.length++;
      }
    }
  }
  while (rooks) {
    uint64_t r = pop_lsb(&rooks);
    AttackSegments ras = slider_reveal_check_mask_segmented(&r, ROOK_ATTACK_OFFSETS, 4, context);
    for (int i = 0; i < 4; i++) {
      if (ras.attacks[i] != 0) {
        result.attacks[result.length].attack = ras.attacks[i];
        result.attacks[result.length].piecePos = r;
        result.length++;
      }
    }
  }
  return result;
}
