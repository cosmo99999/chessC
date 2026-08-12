#include "move.h"
#include "position.h"
#include "stdbool.h"

typedef struct {
  int depth;
  bool isMax;
  int alpha;
  int beta;
  HashHistory hashHistory;
  Position position;
  int count;
  int captures;
  int enPassant;
  int checks;
  int doublechecks;
  int checkMates;
  int promotions;
} SearchParams;

SearchParams get_empty_params(HashHistory *history, Position *newPos, int depth);
bool has_repetition(uint64_t current, HashHistory *hashes, int halfMoveClock, int repetitions, bool postMove);
int phase_weight(Piece piece);
int evaluate(Position *position);
int min_max_search(SearchParams *params);
Move *get_best_move(MoveArr *mArr, Position *position, HashHistory *hashHistory, int depth);
