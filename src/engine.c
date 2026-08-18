#include "engine.h"
#include "helpers.h"
#include "move.h"
#include "pestoTables.h"
#include "position.h"
#include "pthread.h"
#include "uci.h"
#include <bits/pthreadtypes.h>
#include <stdio.h>

pthread_mutex_t thread_results_lock;

void print_perft(PerftParams *perft) {
  printf("nodes searched: %llu \n", (unsigned long long)perft->count);
  printf("captures %d \n", perft->captures);
  printf("checks %d \n", perft->checks);
  printf("enPassant %d \n", perft->enPassant);
  printf("checkmates %d \n", perft->checkMates);
  printf("promotions %d \n", perft->promotions);
  printf("doublechecks %d \n", perft->doublechecks);
}
void copy_back_counts(PerftParams *from, PerftParams *to) {
  to->count += from->count;
  to->checks += from->checks;
  to->captures += from->captures;
  to->enPassant += from->enPassant;
  to->doublechecks += from->doublechecks;
  to->promotions += from->promotions;
  to->checkMates += from->checkMates;
}

void *thread_search_perft(void *args) {
  ThreadArgs *tArgs = (ThreadArgs *)args;
  PerftParams params = get_empty_params(&tArgs->hashHistory, tArgs->position);
  make_move(&tArgs->move, tArgs->position);
  int score = min_max_search_perft(tArgs->depth - 1, tArgs->position, tArgs->isMax, -10000, 10000, &tArgs->hashHistory,
                                   &params);
  pthread_mutex_lock(&thread_results_lock);
  copy_back_counts(&params, tArgs->paramResults);
  tArgs->bestMoves[tArgs->index] = score;
  pthread_mutex_unlock(&thread_results_lock);
  print_move_uci(tArgs->move.from, tArgs->move.to, tArgs->move.promotionPiece);
  printf(": %llu \n", (unsigned long long)params.count);
  return NULL;
}
void *thread_search(void *args) {
  ThreadArgs *tArgs = (ThreadArgs *)args;
  make_move(&tArgs->move, tArgs->position);
  int score = min_max(tArgs->depth - 1, tArgs->position, tArgs->isMax, -10000, 10000, &tArgs->hashHistory);
  pthread_mutex_lock(&thread_results_lock);
  tArgs->bestMoves[tArgs->index] = score;
  pthread_mutex_unlock(&thread_results_lock);
  free(tArgs->position);
  free(tArgs);
  return NULL;
}

Move *perft_get_move(MoveArr *mArr, Position *position, HashHistory *hashHistory, int depth, bool multithread) {
  if (mArr->count == 0) {
    return NULL;
  }
  Move *bestMove = &mArr->moves[0];

  int bestOutcomes[(int)mArr->count];

  PerftParams totals = {0};

  if (multithread) {
    pthread_t threads[mArr->count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t stack_size = 8 * 1024 * 1024;
    pthread_attr_setstacksize(&attr, stack_size);

    for (int i = 0; i < mArr->count; i++) {
      ThreadArgs *tArgs = malloc(sizeof(ThreadArgs));
      Position *newPos = malloc(sizeof(Position));
      *newPos = *position;
      tArgs->position = newPos;
      tArgs->hashHistory = *hashHistory;
      tArgs->move = mArr->moves[i];
      tArgs->isMax = position->whitesMove;
      tArgs->paramResults = &totals;
      tArgs->bestMoves = bestOutcomes;
      tArgs->index = i;
      tArgs->depth = depth;

      pthread_create(&threads[i], &attr, thread_search, (void *)tArgs);
    }

    for (int i = 0; i < mArr->count; i++) {
      pthread_join(threads[i], NULL);
    }
  } else {
    for (int i = 0; i < mArr->count; i++) {
      Position *newPos = malloc(sizeof(Position));
      *newPos = *position;
      make_move(&mArr->moves[i], newPos);
      BoardInfo bInfo;
      bInfo.position = newPos;
      PerftParams params = {0};
      int score = min_max_search_perft(depth - 1, position, position->whitesMove, 10000, -10000, hashHistory, &params);
      pthread_mutex_lock(&thread_results_lock);
      copy_back_counts(&params, &totals);
      bestOutcomes[i] = score;
      pthread_mutex_unlock(&thread_results_lock);
      print_move_uci(mArr->moves[i].from, mArr->moves[i].to, mArr->moves[i].promotionPiece);
      printf(": %llu \n", (unsigned long long)params.count);
    }
  }

  print_perft(&totals);

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
Move *get_move(MoveArr *mArr, Position *position, HashHistory *hashHistory, int depth, bool multithread) {
  if (mArr->count == 0) {
    return NULL;
  }
  Move *bestMove = &mArr->moves[0];

  int bestOutcomes[(int)mArr->count];

  if (multithread) {
    pthread_t threads[mArr->count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    size_t stack_size = 8 * 1024 * 1024;
    pthread_attr_setstacksize(&attr, stack_size);

    for (int i = 0; i < mArr->count; i++) {
      ThreadArgs *tArgs = malloc(sizeof(ThreadArgs));
      Position *newPos = malloc(sizeof(Position));
      *newPos = *position;
      tArgs->position = newPos;
      tArgs->hashHistory = *hashHistory;
      tArgs->move = mArr->moves[i];
      tArgs->isMax = !position->whitesMove;
      tArgs->bestMoves = bestOutcomes;
      tArgs->index = i;
      tArgs->depth = depth;
      pthread_create(&threads[i], &attr, thread_search, (void *)tArgs);
    }

    for (int i = 0; i < mArr->count; i++) {
      pthread_join(threads[i], NULL);
      printf("%d : %d\n", i, bestOutcomes[i]);
    }
  } else {
    for (int i = 0; i < mArr->count; i++) {
      Position *newPos = malloc(sizeof(Position));
      *newPos = *position;
      make_move(&mArr->moves[i], newPos);
      BoardInfo bInfo;
      bInfo.position = newPos;
      int score = min_max(depth - 1, position, !position->whitesMove, -10000, 10000, hashHistory);
      bestOutcomes[i] = score;
    }
  }

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
    print_move(bestMove);
    printf("at index : %d\n", bestMovePosition);
  }
  return bestMove;
}

void count_values(PerftParams *nParams, Move *m, AttackerInfo aInfo, int oldEnPassantTile) {
  if (m->pto != None) {
    nParams->captures++;
  }
  if (m->to == oldEnPassantTile && m->pfrom == Pawn)
    nParams->enPassant++;
  if (aInfo.check) {
    nParams->checks++;
  }
  if (aInfo.multicheck) {
    nParams->doublechecks++;
  }
  if (m->promotion) {
    nParams->promotions++;
  }
}
int min_max_search_perft(int depth, Position *position, bool isMax, int alpha, int beta, HashHistory *hashHistory,
                         PerftParams *params) {
  if (depth == 0) {
    MoveArr mArr = get_moves(position);
    AttackerInfo aInfo = get_attacker_info(position);
    if (mArr.count == 0) {
      if (aInfo.check) {
        params->checkMates++;
        if (isMax) {
          return 1000000;
        } else {
          return -1000000;
        }
      } else {
        return 0;
      }
    }
    if (has_repetition(position->zhash, hashHistory, position->halfMoveClock, 2, false)) {
      return 0;
    }
    int e = evaluate(position);
    return e;
  }

  MoveArr mArr = get_moves(position);
  if (depth == 1) {
    params->count += mArr.count;
  }

  if (isMax) {
    float best = -100000;
    for (int i = 0; i < mArr.count; i++) {

      Move *m = &mArr.moves[i];
      uint64_t oldEnPassantTile = position->enPassantTile;
      UndoMove undo = make_move(m, position);
      AttackerInfo aInfo = get_attacker_info(position);

      // printf("Made move: \n");
      // print_move(m);
      // print_board(info->position);
      // printf("\n");
      hashHistory->hashes[hashHistory->count++] = position->zhash;
      int temp = min_max_search_perft(depth - 1, position, false, alpha, beta, hashHistory, params);
      hashHistory->count--;

      if (depth == 1)
        count_values(params, m, aInfo, oldEnPassantTile);

      unmake_move(m, position, &undo);
      // printf("undoing move: \n");
      // print_move(m);
      // print_board(info->position);
      // printf("\n");
      best = max(best, temp);
      alpha = max(best, alpha);
      // if (alpha >= beta)
      //   break;
    }
    return best;
  }
  if (!isMax) {
    float best = 100000;
    for (int i = 0; i < mArr.count; i++) {
      Move *m = &mArr.moves[i];
      uint64_t oldEnPassantTile = position->enPassantTile;
      UndoMove undo = make_move(m, position);
      AttackerInfo aInfo = get_attacker_info(position);

      // printf("Made move: \n");
      // print_move(m);
      // print_board(info->position);
      // printf("\n");
      hashHistory->hashes[hashHistory->count++] = position->zhash;
      int temp = min_max_search_perft(depth - 1, position, true, alpha, beta, hashHistory, params);
      hashHistory->count--;

      if (depth == 1)
        count_values(params, m, aInfo, oldEnPassantTile);

      unmake_move(m, position, &undo);
      // printf("undoing move: \n");
      // print_move(m);
      // print_board(info->position);
      // printf("\n");

      best = min(best, temp);
      beta = min(best, beta);
      // if (alpha >= beta)
      //   break;
    }
    return best;
  }
  return -1;
}
int min_max(int depth, Position *position, bool isMax, int alpha, int beta, HashHistory *hashHistory) {
  MoveArr mArr = get_moves(position);

  if (mArr.count == 0) {
    MoveArr mArr = get_moves(position);
    AttackerInfo aInfo = get_attacker_info(position);
    if (mArr.count == 0) {
      if (aInfo.check) {
        if (isMax) {
          return -1000000 - depth;
        } else {
          return 1000000 + depth;
        }
      } else {
        return 0;
      }
    }
    if (has_repetition(position->zhash, hashHistory, position->halfMoveClock, 2, false)) {
      return 0;
    }
  }
  if (depth == 0) {
    int e = evaluate(position);
    return e;
  }

  if (isMax) {
    int best = -100000;
    for (int i = 0; i < mArr.count; i++) {

      Move *m = &mArr.moves[i];
      UndoMove undo = make_move(m, position);
      AttackerInfo aInfo = get_attacker_info(position);

      hashHistory->hashes[hashHistory->count++] = position->zhash;
      int temp = min_max(depth - 1, position, false, alpha, beta, hashHistory);
      hashHistory->count--;

      unmake_move(m, position, &undo);
      best = max(best, temp);
      alpha = max(best, alpha);
      if (alpha >= beta) {
        break;
      }
    }
    return best;
  }
  if (!isMax) {
    int best = 100000;
    for (int i = 0; i < mArr.count; i++) {
      Move *m = &mArr.moves[i];
      UndoMove undo = make_move(m, position);
      AttackerInfo aInfo = get_attacker_info(position);

      hashHistory->hashes[hashHistory->count++] = position->zhash;
      int temp = min_max(depth - 1, position, true, alpha, beta, hashHistory);
      hashHistory->count--;

      unmake_move(m, position, &undo);

      best = min(best, temp);
      beta = min(best, beta);
      if (alpha >= beta) {
        break;
      }
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
PerftParams get_empty_params(HashHistory *history, Position *newPos) {

  PerftParams params;
  params.count = 0;
  params.captures = 0;
  params.checks = 0;
  params.enPassant = 0;
  params.doublechecks = 0;
  params.checkMates = 0;
  params.promotions = 0;
  return params;
}
