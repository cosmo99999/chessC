#include "engine.h"
#include "helpers.h"
#include "move.h"
#include "pestoTables.h"
#include "position.h"
#include "uci.h"
#include <stdio.h>

int mg_value[6] = {82, 337, 365, 477, 1025, 0};
int eg_value[6] = {94, 281, 297, 512, 936, 0};

Move *get_best_move(MoveArr *mArr, Position *position, HashHistory *hashHistory, int depth) {
  if (mArr->count == 0) {
    return NULL;
  }
  Move *bestMove = &mArr->moves[0];
  int bestOutcomes[(int)mArr->count];
  int totalMoves = 0;
  int captures = 0;
  int enPassant = 0;
  int checks = 0;
  int checkmates = 0;
  int promotions = 0;
  int multichecks = 0;
  for (int i = 0; i < mArr->count; i++) {
    Position newPos = *position;
    Move *m = &mArr->moves[i];
    make_move(m, &newPos);
    HashHistory history = *hashHistory;
    SearchParams params = get_empty_params(&history, &newPos, depth);
    int score = min_max_search(&params);
    totalMoves += params.count;
    captures += params.captures;
    checks += params.checks;
    enPassant += params.enPassant;
    checkmates += params.checkMates;
    promotions += params.promotions;
    multichecks += params.doublechecks;
    bestOutcomes[i] = score;
    print_move_uci(m->from, m->to, m->promotionPiece);
    printf(": %d \n", params.count);
  }
  printf("nodes searched: %d \n", totalMoves);

  // printf("searched %d \n", totalMoves);
  // printf("captures %d \n", captures);
  // printf("checks %d \n", checks);
  // printf("enPassant %d \n", enPassant);
  // printf("checkmates %d \n", checkmates);
  // printf("promotions %d \n", promotions);
  // printf("doublechecks %d \n", multichecks);
  int best = position->whitesMove ? -10000 : 10000;
  int bestMovePosition = -1;

  // for (int i = 0; i < mArr->count; i++) {
  //   printf("%d: %d \n", i, bestOutcomes[i]);
  // }
  for (int i = 0; i < mArr->count; i++) {

    if (position->whitesMove) {
      if (bestOutcomes[i] > best) {
        best = bestOutcomes[i];
        bestMovePosition = i;
      }
    } else {
      if (bestOutcomes[i] < best) {
        best = bestOutcomes[i];
        bestMovePosition = i;
      }
    }
  }
  if (bestMovePosition != -1) {
    bestMove = &mArr->moves[bestMovePosition];
  }
  return bestMove;
}
SearchParams copy_params(SearchParams *p) {
  SearchParams nParams = *p;
  nParams.count = 0;
  nParams.captures = 0;
  nParams.checks = 0;
  nParams.enPassant = 0;
  nParams.doublechecks = 0;
  nParams.checkMates = 0;
  nParams.promotions = 0;
  if (p->isMax) {
    nParams.isMax = false;
  } else {
    nParams.isMax = true;
  }
  nParams.depth -= 1;
  return nParams;
}
void count_values(SearchParams *nParams, Move *m, AttackerInfo aInfo, int oldEnPassantTile) {
  if (nParams->depth == 0) {
    if (m->pto != None) {
      nParams->captures++;
    }
    if (m->to == oldEnPassantTile && m->pfrom == Pawn)
      nParams->enPassant++;
    if (m->to == aInfo.checkingPiecePos) {
      nParams->checks++;
    }
    if (aInfo.multicheck) {
      nParams->doublechecks++;
    }
    if (m->promotion) {
      nParams->promotions++;
    }
  }
}
void copy_back_counts(SearchParams *from, SearchParams *to) {
  to->count += from->count;
  to->checks += from->checks;
  to->captures += from->captures;
  to->enPassant += from->enPassant;
  to->doublechecks += from->doublechecks;
  to->promotions += from->promotions;
  to->checkMates += from->checkMates;
}
int min_max_search(SearchParams *params) {

  if (params->depth == 0) {
    MoveArr mArr = get_moves(&params->position);
    AttackerInfo aInfo = get_attacker_info(&params->position);
    if (mArr.count == 0) {
      if (aInfo.check) {
        params->checkMates++;
        if (params->isMax) {
          return 1000000;
        } else {
          return -1000000;
        }
      } else {
        return 0;
      }
    }
    if (has_repetition(params->position.zhash, &params->hashHistory, params->position.halfMoveClock, 2, false)) {
      return 0;
    }
    int e = evaluate(&params->position);
    return e;
  }
  MoveArr mArr = get_moves(&params->position);
  if (params->depth == 1) {
    params->count += mArr.count;
  }

  if (params->isMax) {
    float best = -100000;
    for (int i = 0; i < mArr.count; i++) {
      SearchParams nParams = copy_params(params);

      Move *m = &mArr.moves[i];
      uint64_t oldEnPassantTile = nParams.position.enPassantTile;
      make_move(m, &nParams.position);
      AttackerInfo aInfo = get_attacker_info(&nParams.position);

      params->hashHistory.hashes[params->hashHistory.count++] = nParams.position.zhash;
      int temp = min_max_search(&nParams);
      params->hashHistory.count--;

      count_values(&nParams, m, aInfo, oldEnPassantTile);
      copy_back_counts(&nParams, params);

      best = max(best, temp);
      params->alpha = max(best, params->alpha);
      // if (params->alpha >= params->beta)
      //   break;
    }
    return best;
  }
  if (!params->isMax) {
    float best = 100000;
    for (int i = 0; i < mArr.count; i++) {
      SearchParams nParams = copy_params(params);

      uint64_t oldEnPassantTile = nParams.position.enPassantTile;
      Move *m = &mArr.moves[i];
      make_move(m, &nParams.position);
      AttackerInfo aInfo = get_attacker_info(&nParams.position);

      params->hashHistory.hashes[params->hashHistory.count++] = nParams.position.zhash;
      int temp = min_max_search(&nParams);
      params->hashHistory.count--;

      count_values(&nParams, m, aInfo, oldEnPassantTile);
      copy_back_counts(&nParams, params);

      best = min(best, temp);
      params->beta = min(best, params->beta);
      // if (params->alpha >= params->beta)
      //   break;
    }
    return best;
  }
  return -1;
}
int phaseWeight(Piece piece) {
  switch (piece) {
  case Pawn:
    return 0;
  case Bishop:
    return 1;
  case Knight:
    return 1;
  case Rook:
    return 2;
  case Queen:
    return 4;
  case King:
    return 0;
  default:
    return 0;
  }
}
bool has_repetition(uint64_t current, HashHistory *hashes, int halfMoveClock, int repetitions, bool postMove) {

  int size = hashes->count;
  int limit = size - halfMoveClock;

  if (limit < 0)
    limit = 0;

  int count = 0;

  int offset = postMove ? 3 : 2;

  for (int i = size - offset; i >= limit; i -= 2) {
    if (hashes->hashes[i] == current) {
      count++;
    }
    if (count >= repetitions) {
      return true;
    }
  }

  return false;
}
int evaluate(Position *position) {

  int pWeight = 0;
  int egScore = 0;
  int mgScore = 0;

  uint64_t wpawns = position->wpawns;
  uint64_t bpawns = position->bpawns;
  uint64_t wbishops = position->wbishops;
  uint64_t bbishops = position->bbishops;
  uint64_t wknights = position->wknights;
  uint64_t bknights = position->bknights;
  uint64_t wrooks = position->wrooks;
  uint64_t brooks = position->brooks;
  uint64_t wqueens = position->wqueens;
  uint64_t bqueens = position->bqueens;
  uint64_t wking = position->wking;
  uint64_t bking = position->bking;

  while (wpawns) {
    int p = pop_lsb_get_int(&wpawns) ^ 56;
    int egval = eg_pesto_table[0][0][p] + eg_value[0];
    int mgVal = mg_pesto_table[0][0][p] + mg_value[0];
    egScore += eg_pesto_table[0][0][p] + eg_value[0];
    mgScore += mg_pesto_table[0][0][p] + mg_value[0];
  }
  while (bpawns) {
    int p = pop_lsb_get_int(&bpawns) ^ 56;
    egScore -= eg_pesto_table[1][0][p] + eg_value[0];
    mgScore -= mg_pesto_table[1][0][p] + mg_value[0];
  }
  while (wbishops) {
    int p = pop_lsb_get_int(&wbishops) ^ 56;
    pWeight += phaseWeight(Bishop);
    egScore += eg_pesto_table[0][2][p] + eg_value[2];
    mgScore += mg_pesto_table[0][2][p] + mg_value[2];
  }
  while (bbishops) {
    int p = pop_lsb_get_int(&bbishops) ^ 56;
    pWeight += phaseWeight(Bishop);
    egScore -= eg_pesto_table[1][2][p] + eg_value[2];
    mgScore -= mg_pesto_table[1][2][p] + mg_value[2];
  }
  while (wknights) {
    int p = pop_lsb_get_int(&wknights) ^ 56;
    pWeight += phaseWeight(Knight);
    egScore += eg_pesto_table[0][1][p] + eg_value[1];
    mgScore += mg_pesto_table[0][1][p] + mg_value[1];
  }
  while (bknights) {
    int p = pop_lsb_get_int(&bknights) ^ 56;
    pWeight += phaseWeight(Knight);
    egScore -= eg_pesto_table[1][1][p] + eg_value[1];
    mgScore -= mg_pesto_table[1][1][p] + mg_value[1];
  }
  while (wrooks) {
    int p = pop_lsb_get_int(&wrooks) ^ 56;
    pWeight += phaseWeight(Rook);
    egScore += eg_pesto_table[0][3][p] + eg_value[3];
    mgScore += mg_pesto_table[0][3][p] + mg_value[3];
  }
  while (brooks) {
    int p = pop_lsb_get_int(&brooks) ^ 56;
    pWeight += phaseWeight(Rook);
    egScore -= eg_pesto_table[1][3][p] + eg_value[3];
    mgScore -= mg_pesto_table[1][3][p] + mg_value[3];
  }
  while (wqueens) {
    int p = pop_lsb_get_int(&wqueens) ^ 56;
    pWeight += phaseWeight(Queen);
    egScore += eg_pesto_table[0][4][p] + eg_value[4];
    mgScore += mg_pesto_table[0][4][p] + mg_value[4];
  }
  while (bqueens) {
    int p = pop_lsb_get_int(&bqueens) ^ 56;
    pWeight += phaseWeight(Queen);
    egScore -= eg_pesto_table[1][4][p] + eg_value[4];
    mgScore -= mg_pesto_table[1][4][p] + mg_value[4];
  }
  int wk = pop_lsb_get_int(&wking) ^ 56;
  pWeight += phaseWeight(King);
  egScore += eg_pesto_table[0][5][wk];
  mgScore += mg_pesto_table[0][5][wk];

  int bk = pop_lsb_get_int(&bking) ^ 56;
  pWeight += phaseWeight(King);
  egScore -= eg_pesto_table[1][5][bk];
  mgScore -= mg_pesto_table[1][5][bk];
  // early promotion catch
  if (pWeight > 24)
    pWeight = 24;
  int egPhase = 24 - pWeight;
  int mgPhase = pWeight;
  int result = (mgScore * mgPhase + egScore * egPhase) / 24;
  return result;
}
SearchParams get_empty_params(HashHistory *history, Position *newPos, int depth) {

  SearchParams params;
  params.hashHistory = *history;
  params.isMax = newPos->whitesMove ? true : false;
  params.alpha = -10000;
  params.beta = 10000;
  params.position = *newPos;
  params.depth = depth - 1;
  params.count = 0;
  params.captures = 0;
  params.checks = 0;
  params.enPassant = 0;
  params.doublechecks = 0;
  params.checkMates = 0;
  params.promotions = 0;
  return params;
}
