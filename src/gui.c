#include "gui.h"
#include "engine.h"
#include "helpers.h"
#include "position.h"
#include <raylib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

Textures load_textures() {
  Textures t;
  char wpawn[50] = ASSET_DIR;
  strncat(wpawn, "whitepawn.png", sizeof(wpawn) - strlen(wpawn) - 1);
  char bpawn[50] = ASSET_DIR;
  strncat(bpawn, "blackpawn.png", sizeof(bpawn) - strlen(bpawn) - 1);
  char wknight[50] = ASSET_DIR;
  strncat(wknight, "whiteknight.png", sizeof(wknight) - strlen(wknight) - 1);
  char bknight[50] = ASSET_DIR;
  strncat(bknight, "blackknight.png", sizeof(bknight) - strlen(bknight) - 1);
  char bbishop[50] = ASSET_DIR;
  strncat(bbishop, "blackbishop.png", sizeof(bbishop) - strlen(bbishop) - 1);
  char wbishop[50] = ASSET_DIR;
  strncat(wbishop, "whitebishop.png", sizeof(wbishop) - strlen(wbishop) - 1);
  char wrook[50] = ASSET_DIR;
  strncat(wrook, "whiterook.png", sizeof(wrook) - strlen(wrook) - 1);
  char brook[50] = ASSET_DIR;
  strncat(brook, "blackrook.png", sizeof(brook) - strlen(brook) - 1);
  char bqueen[50] = ASSET_DIR;
  strncat(bqueen, "blackqueen.png", sizeof(bqueen) - strlen(bqueen) - 1);
  char wqueen[50] = ASSET_DIR;
  strncat(wqueen, "whitequeen.png", sizeof(wqueen) - strlen(wqueen) - 1);
  char wking[50] = ASSET_DIR;
  strncat(wking, "whiteking.png", sizeof(wking) - strlen(wking) - 1);
  char bking[50] = ASSET_DIR;
  strncat(bking, "blackking.png", sizeof(bking) - strlen(bking) - 1);
  printf("%s\n", wpawn);
  printf("%s\n", bpawn);
  t.whitePawn = LoadTexture(wpawn);
  t.blackPawn = LoadTexture(bpawn);
  t.whiteBishop = LoadTexture(wbishop);
  t.blackBishop = LoadTexture(bbishop);
  t.blackKnight = LoadTexture(bknight);
  t.whiteKnight = LoadTexture(wknight);
  t.blackRook = LoadTexture(brook);
  t.whiteRook = LoadTexture(wrook);
  t.whiteQueen = LoadTexture(wqueen);
  t.blackQueen = LoadTexture(bqueen);
  t.whiteKing = LoadTexture(wking);
  t.blackKing = LoadTexture(bking);
  return t;
}

Texture2D *get_texture(Piece piece, Colour colour, Textures *textures) {
  if (colour == White) {
    if (piece == Pawn) {
      return &textures->whitePawn;
    }
    if (piece == Bishop) {
      return &textures->whiteBishop;
    }
    if (piece == Knight) {
      return &textures->whiteKnight;
    }
    if (piece == Rook) {
      return &textures->whiteRook;
    }
    if (piece == Queen) {
      return &textures->whiteQueen;
    }
    if (piece == King) {
      return &textures->whiteKing;
    }
  } else {
    if (piece == Pawn) {
      return &textures->blackPawn;
    }
    if (piece == Bishop) {
      return &textures->blackBishop;
    }
    if (piece == Knight) {
      return &textures->blackKnight;
    }
    if (piece == Rook) {
      return &textures->blackRook;
    }
    if (piece == Queen) {
      return &textures->blackQueen;
    }
    if (piece == King) {
      return &textures->blackKing;
    }
  }
  return NULL;
}

int Sq(int x, int y) { return y * 8 + x; }

DrawContext get_init_context() {
  DrawContext d;
  d.firstSelection = -1;
  d.secondSelection = -1;
  d.gameState = WhiteMove;
  d.promotionChoice = None;
  return d;
}
void mouse_check(DrawContext *context) {
  int boardStartX = BOARD_X;
  int boardEndX = BOARD_X + BOARD_WIDTH;
  int boardStartY = BOARD_Y;
  int boardEndY = BOARD_Y + BOARD_HEIGHT;
  Vector2 mousePos = GetMousePosition();
  int tileMousePos = -1;

  // printf("mouse x: %f \n", mousePos.x);
  // printf("mouse y: %f \n", mousePos.y);
  if (context->gameState == Promoting) {
    if (IsMouseButtonPressed(0)) {
      int promotionMenuStartY = BOARD_Y;
      if (mousePos.x > boardEndX && mousePos.x < boardEndX + 100) {
        if (mousePos.y > promotionMenuStartY && mousePos.y < promotionMenuStartY + 100)
          context->promotionChoice = Queen;
        else if (mousePos.y > promotionMenuStartY + 100 && mousePos.y < promotionMenuStartY + 200)
          context->promotionChoice = Bishop;
        else if (mousePos.y > promotionMenuStartY + 200 && mousePos.y < promotionMenuStartY + 300)
          context->promotionChoice = Rook;
        else if (mousePos.y > promotionMenuStartY + 300 && mousePos.y < promotionMenuStartY + 400)
          context->promotionChoice = Knight;
      }
    }
  } else {
    if (mousePos.x > boardStartX && mousePos.x < boardEndX && mousePos.y > boardStartY && mousePos.y < boardEndY) {
      float mx = mousePos.x - BOARD_X;
      float my = mousePos.y - BOARD_Y - 100;
      // printf("mx: %f \n", mx);
      // printf("my: %f \n", my);
      tileMousePos = Sq(mx / 100, 7 - (my / 100));
    }
  }
  if (IsMouseButtonPressed(0) && tileMousePos != -1) {
    if (context->firstSelection == -1) {
      for (int i = 0; i < context->mArr->count; i++) {
        Move *m = &context->mArr->moves[i];
        uint64_t ullFrom = 1ULL << tileMousePos;
        if (m->from == ullFrom) {
          context->firstSelection = tileMousePos;
        }
      }
    } else {
      context->secondSelection = tileMousePos;
      printf("setting second pos: %d \n", tileMousePos);
    }
  }
}
void DrawPiece(uint64_t piece, DrawContext context, Texture2D *texture) {
  int pieceSqPos = lsb_get_int(&piece);
  int x = pieceSqPos & 7;
  int y = pieceSqPos >> 3;
  if (context.firstSelection == pieceSqPos)
    DrawTexture(*texture, context.mousePosition.x - 50, context.mousePosition.y - 50, WHITE);
  else {
    DrawTexture(*texture, x * 100 + BOARD_X, 700 - y * 100 + BOARD_Y, WHITE);
  }
}

void draw_gui(Position *position, MoveArr *moves, DrawContext *context) {
  int counter = 0;
  char counterStr[10];
  MoveArr currentPieceMoves;
  currentPieceMoves.count = 0;

  if (context->firstSelection != -1) {
    for (int i = 0; i < moves->count; i++) {
      Move *m = &moves->moves[i];
      int mFrom = lsb_get_int(&m->from);
      if (mFrom == context->firstSelection) {
        currentPieceMoves.moves[currentPieceMoves.count] = *m;
        currentPieceMoves.count++;
      }
    }
  }
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      int drawPos[2];
      drawPos[0] = BOARD_X + j * 100;
      drawPos[1] = BOARD_Y + 700 - (i * 100);
      Color c = BLACK;
      if (i % 2 == 0) {
        if (j % 2 == 0)
          c = BROWN;
        else
          c = LIGHTGRAY;
      } else {
        if (j % 2 == 0)
          c = LIGHTGRAY;
        else
          c = BROWN;
      }
      snprintf(counterStr, sizeof(counterStr), "%d", counter);
      DrawRectangle(drawPos[0], drawPos[1], 100, 100, c);

      for (int i = 0; i < currentPieceMoves.count; i++) {
        Move *m = &currentPieceMoves.moves[i];
        int mPos = lsb_get_int(&m->to);
        if (mPos == counter) {
          DrawRectangle(drawPos[0], drawPos[1], 100, 100, GREEN);
        }
      }
      DrawText(counterStr, drawPos[0] + 10, drawPos[1] + 10, 10, RED);
      counter++;
    }
  }
  uint64_t allOccupancy = white_piece_mask(position) | black_piece_mask(position);

  while (allOccupancy) {
    uint64_t piece = pop_lsb(&allOccupancy);
    Piece type = find_piece_type(piece, position);
    Colour colour = find_piece_colour(piece, position);
    Texture2D *texture = get_texture(type, colour, context->textures);
    DrawPiece(piece, *context, texture);
  }

  int currentEval = evaluate(position);
  char eval[100];
  snprintf(eval, sizeof(eval), "%d", currentEval);
  DrawText(eval, BOARD_X + BOARD_WIDTH + 50, BOARD_Y + 100, 10, RED);

  if (context->gameState == WhiteMove) {
    DrawText("WhiteMove", (BOARD_X + BOARD_WIDTH) / 2, BOARD_Y + BOARD_HEIGHT + 50, 10, RED);
  }

  if (context->gameState == BlackMove) {
    DrawText("BlackMove", (BOARD_X + BOARD_WIDTH) / 2, BOARD_Y + BOARD_HEIGHT + 50, 10, RED);
  }
  if (context->gameState == WhiteWon) {
    DrawText("WhiteWon", (BOARD_X + BOARD_WIDTH) / 2, BOARD_Y + BOARD_HEIGHT + 50, 10, RED);
  }
  if (context->gameState == BlackWon) {
    DrawText("BlackWon", (BOARD_X + BOARD_WIDTH) / 2, BOARD_Y + BOARD_HEIGHT + 50, 10, RED);
  }
  if (context->gameState == StaleMate) {
    DrawText("StaleMate", (BOARD_X + BOARD_WIDTH) / 2, BOARD_Y + BOARD_HEIGHT + 50, 10, RED);
  }
}
