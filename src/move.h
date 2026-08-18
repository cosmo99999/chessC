#pragma once
#include "position.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FILECONSTANTS
#define FILECONSTANTS

#define A_FILE 72340172838076673ULL
#define B_FILE 144680345676153346ULL
#define C_FILE 289360691352306692ULL
#define D_FILE 578721382704613384ULL
#define E_FILE 1157442765409226768ULL
#define F_FILE 2314885530818453536ULL
#define G_FILE 4629771061636907072ULL
#define H_FILE 9259542123273814144ULL

#define ROW_ONE 255ULL
#define ROW_TWO 65280ULL
#define ROW_THREE 16711680ULL
#define ROW_FOUR 4278190080ULL
#define ROW_FIVE 1095216660480ULL
#define ROW_SIX 280375465082880ULL
#define ROW_SEVEN 71776119061217280ULL
#define ROW_EIGHT 18374686479671623680ULL

#endif

#ifndef SLIDER_OFFSETS
#define SLIDER_OFFSETS
static int QUEEN_ATTACK_OFFSETS[8] = {-1, 1, 8, -8, 7, -7, 9, -9};
static int ROOK_ATTACK_OFFSETS[4] = {-1, 1, 8, -8};
static int BISHOP_ATTACK_OFFSETS[4] = {7, -7, 9, -9};
#endif

typedef struct {
  uint64_t attack;
  uint64_t piecePos;
} RevealCheckAttack;

typedef struct {
  RevealCheckAttack attacks[8];
  int length;
} RevealAttackSegments;

typedef struct {
  uint64_t attacks[8];
  int length;
} AttackSegments;

typedef struct {
  uint64_t friendlyPieces;
  uint64_t attackingPieces;
  uint64_t king;
} PositionContext;
typedef struct {
  bool check;
  bool multicheck;
  uint64_t checkingPiecePos;
  uint64_t checkMask;
  uint64_t fullAttackMask;
} AttackerInfo;

void sort_moves(MoveArr *mArr);

MoveArr get_moves(Position *position);
// illegal move filtering
AttackerInfo get_attacker_info(Position *position);
RevealAttackSegments get_reveal_check_lines(Position *position);

// pawns
void pawn_moves(MoveArr *moves, Position *position);
uint64_t wpawn_attack_mask(uint64_t *pawn);
uint64_t wpawn_move_mask(uint64_t *pawn, uint64_t enemyPosMask);
uint64_t bpawn_attack_mask(uint64_t *pawn);
uint64_t bpawn_move_mask(uint64_t *pawn, uint64_t enemyPosMask);

// knights
void knight_moves(MoveArr *moves, Position *position);
uint64_t knight_attack_mask(uint64_t *knight);

// sliding pieces
void slider_moves(MoveArr *moves, Position *position, Piece piece);
uint64_t slider_attack_mask(uint64_t piece, uint64_t allpieces, int offsets[], int length);
AttackSegments slider_attack_mask_segmented(uint64_t *piece, uint64_t allpieces, int offsets[], int length);
AttackSegments slider_reveal_check_mask_segmented(uint64_t *piece, int offsets[], int length, PositionContext context);

// king
void king_moves(MoveArr *moves, Position *position, AttackerInfo *aInfo);
uint64_t king_attack_mask(uint64_t *king);
