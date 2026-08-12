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

void UCI() {
  while (true) {
    char input[1000];
    int word_count = 0;
    read_input(input);
    char **words = split_by_whitespace(input, &word_count);

    if (word_count == 0)
      continue;

    if (equals(words[0], "position")) {
      int idx = 1;
      if (equals(words[idx], "startpos")) {
        *position = start_position();
        printf("hit move init \n");
        idx++;
      } else if (equals(words[idx], "fen")) {
        // fen string implementation
      }
      *mArr = get_moves(position);

      if (idx < word_count && equals(words[idx], "moves")) {
        for (int j = idx; j < word_count; j++) {
          ParsedMove pm = parse_uci_move(words[j]);
          Move *actualMove;
          for (int z = 0; z < mArr->count; z++) {
            Move *m = &mArr->moves[z];
            if (m->from == pm.from && m->to == pm.to) {
              if (pm.promotionPiece != None) {
                if (m->promotionPiece == pm.promotionPiece) {
                  actualMove = m;
                }
              } else {
                actualMove = m;
                *mArr = get_moves(position);
              }
            }
          }
          make_move(actualMove, position);
        }
      }
    } else if (equals(words[0], "go")) {
      for (int j = 1; j < word_count; j++) {
        char *word2 = words[j];
        if (equals(word2, "perft")) {
          char *word3 = words[j + 1];
          int depth = atoi(word3);
          get_best_move(mArr, position, hashHistory, depth);
        }
      }
    }
    print_board(position);
  }
}

void GUI() {

  *position = start_position();
  InitWindow(1000, 1000, "ChessTwo");
  SetTargetFPS(240);

  MoveArr moves = get_moves(position);
  textures = malloc(sizeof(Textures));
  *textures = load_textures();

  DrawContext context = get_init_context();
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

    if (context.gameState == WhiteMove) {
      Move *m = get_best_move(&moves, position, hashHistory, 4);
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
    if (context.firstSelection != -1 && context.secondSelection != -1) {
      for (int i = 0; i < moves.count; i++) {
        Move *m = &moves.moves[i];
        uint64_t bitFrom = 1ULL << context.firstSelection;
        uint64_t bitTo = 1ULL << context.secondSelection;
        if (m->from == bitFrom && m->to == bitTo) {
          make_move(m, position);
          hashHistory->hashes[hashHistory->count++] = position->zhash;
          moves.count = 0;
          moves = get_moves(position);
          context.mArr = &moves;
          context.gameState = position->whitesMove ? WhiteMove : BlackMove;
        }
      }
      context.firstSelection = -1;
      context.secondSelection = -1;
    }
    draw_gui(position, &moves, &context);
    EndDrawing();
  }
}

int main(int argc, char *argv[]) {

  zobrist = malloc(sizeof(Zobrist));
  *zobrist = init_zobrist();

  hashHistory = malloc(sizeof(HashHistory));
  position = malloc(sizeof(Position));

  mArr = malloc(sizeof(MoveArr));
  printf("uci or gui\n");
  char buffer[256];
  read_input(buffer);

  if (equals(buffer, "uci")) {
    UCI();
  } else {
    GUI();
  }

  return 0;
}
