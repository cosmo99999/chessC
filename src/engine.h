#include "move.h"
#include "position.h"
#include "stdbool.h"

typedef struct {
  uint64_t count;
  int captures;
  int enPassant;
  int checks;
  int doublechecks;
  int checkMates;
  int promotions;
} PerftParams;

typedef struct {
  Position *position;
  AttackerInfo *aInfo;
} BoardInfo;

typedef struct {
  PerftParams *paramResults;
  Position *position;
  Move move;
  HashHistory hashHistory;
  bool isMax;
  int depth;
  int *bestMoves;
  int index;
} ThreadArgs;

Move *perft_get_move(MoveArr *mArr, Position *position, HashHistory *hashHistory, int depth, bool multithread);
Move *get_move(MoveArr *mArr, Position *position, HashHistory *hashHistory, int depth, bool multithread);

PerftParams get_empty_params(HashHistory *history, Position *newPos);

bool has_repetition(uint64_t current, HashHistory *hashes, int halfMoveClock, int repetitions, bool postMove);
int phase_weight(Piece piece);
int evaluate(Position *position);
int min_max_search_perft(int depth, Position *position, bool isMax, int alpha, int beta, HashHistory *hashHistory,

                         PerftParams *perftParams);

int min_max(int depth, Position *position, bool isMax, int alpha, int beta, HashHistory *hashHistory);
