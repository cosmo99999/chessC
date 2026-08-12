#include "move.h"
#include "position.h"
#include "raylib.h"

#define ASSET_DIR "textures/"
#define BOARD_Y 100
#define BOARD_X 100
#define BOARD_WIDTH 800
#define BOARD_HEIGHT 800

typedef struct {
  Texture2D whitePawn;
  Texture2D blackPawn;
  Texture2D whiteBishop;
  Texture2D blackBishop;
  Texture2D blackKnight;
  Texture2D whiteKnight;
  Texture2D blackRook;
  Texture2D whiteRook;
  Texture2D whiteQueen;
  Texture2D blackQueen;
  Texture2D whiteKing;
  Texture2D blackKing;
} Textures;

Textures load_textures();

typedef enum {
  Promoting,
  WhiteMove,
  BlackMove,
  WhiteWon,
  BlackWon,
  StaleMate,
} GameState;

typedef struct {
  int firstSelection;
  int secondSelection;
  Textures *textures;
  Vector2 mousePosition;
  GameState gameState;
  Piece promotionChoice;
  MoveArr *mArr;
} DrawContext;

DrawContext get_init_context();
void mouse_check(DrawContext *context);
void draw_gui(Position *position, MoveArr *moves, DrawContext *context);
