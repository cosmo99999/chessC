#include "engine.h"
#include "gui.h"
#include "helpers.h"
#include "move.h"
#include "position.h"
#include "uci.h"
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Position *position;
MoveArr *mArr;
Zobrist *zobrist;
Textures *textures;
HashHistory *hashHistory;

int depth = 7;

int read_depth_from_config() {
  FILE *file_ptr;

  char buffer[256];
  file_ptr = fopen("config.txt", "r");
  if (file_ptr == NULL) {
    printf("couldnt read config\n");
    return 7;
  }
  while (fgets(buffer, sizeof(buffer), file_ptr) != NULL) {
    int length = sizeof(buffer) / sizeof(char);
    for (int i = 0; i < length; i++) {
      if (buffer[i] == '=') {
        return atoi(&buffer[i + 1]);
      }
    }
  };
  return 7;
}
void GUI() {

  InitWindow(1000, 1000, "ChessTwo");
  SetTargetFPS(240);

  MoveArr moves = get_moves(position);

  for (int i = 0; i < mArr->count; i++) {
    print_move(&mArr->moves[i]);
  }
  textures = malloc(sizeof(Textures));
  *textures = load_textures();

  DrawContext context = get_init_context();
  context.depth = depth;
  context.gameState = position->whitesMove ? WhiteMove : BlackMove;
  context.textures = textures;
  context.mArr = &moves;

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (moves.count == 0) {
      AttackerInfo aInfo = get_attacker_info(position);
      if (aInfo.check) {
        context.gameState = position->whitesMove ? BlackWon : WhiteWon;
      } else {
        context.gameState = StaleMate;
      }
    }

    if (context.gameState == BlackMove) {
      Move *m = get_move(&moves, position, hashHistory, depth, true);
      context.lastMove = *m;
      make_move(m, position);
      context.gameState = position->whitesMove ? WhiteMove : BlackMove;
      hashHistory->hashes[hashHistory->count++] = position->zhash;
      moves.count = 0;
      moves = get_moves(position);
      context.mArr = &moves;
    }

    bool repetition = has_repetition(position->zhash, hashHistory, position->halfMoveClock, 3, true);
    if (repetition) {
      context.gameState = StaleMate;
    }

    context.mousePosition = GetMousePosition();
    mouse_check(&context);
    if (context.promotionMade) {
      context.lastMove = *context.promotionChoice;
      make_move(context.promotionChoice, position);
      hashHistory->hashes[hashHistory->count++] = position->zhash;
      moves.count = 0;
      moves = get_moves(position);
      context.mArr = &moves;
      context.gameState = position->whitesMove ? WhiteMove : BlackMove;
      context.promotionMade = false;
    }
    if (context.firstSelection != -1 && context.secondSelection != -1) {
      for (int i = 0; i < moves.count; i++) {
        Move *m = &moves.moves[i];
        uint64_t bitFrom = 1ULL << context.firstSelection;
        uint64_t bitTo = 1ULL << context.secondSelection;
        if (m->from == bitFrom && m->to == bitTo) {
          if (m->promotionPiece != None) {
            context.gameState = Promoting;
            context.promotionChoice = m;
          } else {
            context.lastMove = *m;
            make_move(m, position);
            context.firstmove = false;
            hashHistory->hashes[hashHistory->count++] = position->zhash;
            moves.count = 0;
            moves = get_moves(position);
            context.mArr = &moves;
            context.gameState = position->whitesMove ? WhiteMove : BlackMove;
          }
        }
      }
      context.firstSelection = -1;
      context.secondSelection = -1;
    }
    draw_gui(position, &moves, &context);
    EndDrawing();
  }
}

void UCI() {
  while (true) {
    char input[1000];
    int word_count = 0;
    read_input(input, sizeof(input));
    char **words = split_by_whitespace(input, &word_count);

    if (word_count == 0)
      continue;

    if (equals(words[0], "gui")) {
      GUI();
    }
    if (equals(words[0], "position")) {
      int idx = 1;
      if (equals(words[idx], "startpos")) {
        *position = start_position();
        printf("hit move init \n");
        idx++;
      } else if (equals(words[idx], "fen")) {
        idx++;
        int fen_start = idx;
        while (idx < word_count && !equals(words[idx], "moves")) {
          idx++;
        }
        char fenBuf[128] = {0};
        for (int k = fen_start; k < idx; k++) {
          strcat(fenBuf, words[k]);
          if (k != idx - 1)
            strcat(fenBuf, " ");
        }
        printf("%s\n", fenBuf);
        *position = position_from_fen(fenBuf);
      }
      *mArr = get_moves(position);

      if (idx < word_count && equals(words[idx], "moves")) {
        for (int j = idx; j < word_count; j++) {
          ParsedMove pm = parse_uci_move(words[j]);
          Move *actualMove;
          printf("promotionPiece: %d", pm.promotionPiece);
          for (int z = 0; z < mArr->count; z++) {
            Move *m = &mArr->moves[z];
            if (m->from == pm.from && m->to == pm.to) {
              if (pm.promotionPiece != None) {
                if (m->promotionPiece == pm.promotionPiece) {
                  actualMove = m;
                  make_move(actualMove, position);
                  *mArr = get_moves(position);
                }
              } else {
                actualMove = m;
                make_move(actualMove, position);
                *mArr = get_moves(position);
              }
            }
          }
        }
      }
      print_board(position);
    } else if (equals(words[0], "go")) {
      for (int j = 1; j < word_count; j++) {
        char *word2 = words[j];
        if (equals(word2, "perft")) {
          char *word3 = words[j + 1];
          int depth = atoi(word3);
          bool multithreaded = true;
          perft_get_move(mArr, position, hashHistory, depth, multithreaded);
        }
      }
    }
    // print_board(position);
  }
}

int main(int argc, char *argv[]) {

  zobrist = malloc(sizeof(Zobrist));
  *zobrist = init_zobrist();

  hashHistory = malloc(sizeof(HashHistory));
  hashHistory->count = 0;
  position = malloc(sizeof(Position));

  printf("uci or gui\n");
  char buffer[256];
  read_input(buffer, sizeof(buffer));
  depth = read_depth_from_config();
  mArr = malloc(sizeof(MoveArr));
  *position = start_position();
  *mArr = get_moves(position);
  if (equals(buffer, "uci")) {
    UCI();
  } else {
    GUI();
  }

  return 0;
}
