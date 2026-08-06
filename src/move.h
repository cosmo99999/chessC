#pragma once
#include "position.h"
#include <stdint.h>

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

MoveArr get_moves(Position *position);

void pawn_moves(MoveArr *moves, Position *position);
uint64_t wpawn_attack_mask(uint64_t *pawn);
uint64_t wpawn_move_mask(uint64_t *pawn, uint64_t enemyPosMask);
uint64_t bpawn_attack_mask(uint64_t *pawn);
uint64_t bpawn_move_mask(uint64_t *pawn, uint64_t enemyPosMask);

void knight_moves(MoveArr *moves, Position *position);
uint64_t knight_attack_mask(uint64_t *knight);
