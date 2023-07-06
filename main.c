#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "include/raylib.h"
#include "include/raymath.h"

typedef struct Entity 
{
  Vector2 position;
  Rectangle hitbox;
} Entity;

// ######################

const char* win_title = "Flappy Bird";
Vector2 win_screen = (Vector2){288.0f, 512.0f};

// ######################

#define SCROLL_SPEED 110.0f
#define PIPE_TOTAL 3
#define PIPE_GAP 40
#define PIPE_SPACE 190
#define BIRD_GRAVITY 20.0f
#define BIRD_JUMP 5.0f

RenderTexture rt_buffer;
Texture t_bird;
Texture t_pipe;
Texture t_background_day;
Texture t_background_night;
Texture t_foreground;

bool b_debugDraw = false;
bool b_gameover = false;

Entity* p_bird;
Entity* p_pipe[PIPE_TOTAL];
float* p_lastPosX;

float bg_scroll;
float fg_scroll;
float bird_velocity;

// ######################

// main loop thingy
void MainLoop();
void Render();
void RenderDebug();
void UpdateInput();
void UpdateLogic();

// preload/unload assets here
void LoadEverything();
void UnloadEverything();

// reset all position
void Restart();

int main()
{
  SetConfigFlags(
    FLAG_WINDOW_RESIZABLE | 
    FLAG_VSYNC_HINT | 
    FLAG_MSAA_4X_HINT);
    
  InitWindow(win_screen.x, win_screen.y, TextFormat("%s - sillysagiri fork!", win_title));
  SetWindowMinSize(win_screen.x*0.4f, win_screen.y*0.4f);
  SetExitKey(-1);
  SetTargetFPS(30); // use vsync instead

  MaximizeWindow();
  
  // ## initialize
  LoadEverything();
  Restart();

  // ## initialize
  while (!WindowShouldClose()) MainLoop();

  // ## initialize
  UnloadEverything();
  CloseWindow();
  return 0;
}

void LoadEverything()
{
  // TODO: use better random generator
  SetRandomSeed(time(0));

  rt_buffer = LoadRenderTexture(win_screen.x, win_screen.y);
  SetTextureFilter(rt_buffer.texture, TEXTURE_FILTER_POINT);

  t_bird = LoadTexture("Resource/sprites/yellowbird-midflap.png");
  t_pipe = LoadTexture("Resource/sprites/pipe-green.png");
  t_background_day = LoadTexture("Resource/sprites/background-day.png");
  t_background_night = LoadTexture("Resource/sprites/background-night.png");
  t_foreground = LoadTexture("Resource/sprites/base.png");

  p_bird = malloc(sizeof(Entity));
  p_bird->hitbox = (Rectangle){0.0f, 0.0f, t_bird.width, t_bird.height};

  // TODO: refactor pipe array
  for(int i=0; i<PIPE_TOTAL; i++)
  {
    p_pipe[i] = malloc(sizeof(Entity));
    p_pipe[i]->hitbox = (Rectangle){0.0f, 0.0f, t_pipe.width, t_pipe.height};
  }
}

void UnloadEverything()
{
  UnloadRenderTexture(rt_buffer);

  UnloadTexture(t_bird);
  UnloadTexture(t_pipe);
  UnloadTexture(t_background_day);
  UnloadTexture(t_background_night);
  UnloadTexture(t_foreground);

  free(p_bird);
  for(int i=0; i<PIPE_TOTAL; i++) free(p_pipe[i]);
}

void Restart()
{
  b_gameover = false;

  // reset bird position
  p_bird->position = (Vector2){75.0f, win_screen.y * 0.5f - 50.0f};
  bird_velocity = 0.0f;

  // pipe start from offscreen
  int tempX = win_screen.x + 50;
  for(int i=0; i<PIPE_TOTAL; i++)
  {
    int tempY = GetRandomValue(150, win_screen.y - t_foreground.height - 40);
    p_pipe[i]->position = (Vector2){tempX, tempY};
    p_lastPosX = &p_pipe[i]->position.x; // track last x position
    tempX += PIPE_SPACE;
  }
}

void UpdateInput()
{
  if (IsKeyReleased(KEY_ENTER)) b_debugDraw = !b_debugDraw;
  if (IsKeyPressed(KEY_SPACE)) bird_velocity = -BIRD_JUMP;
  if (IsKeyPressed(KEY_R)) Restart();
}

void UpdateLogic()
{
  if (!b_gameover)
  {
    bg_scroll += ((int)(SCROLL_SPEED * 0.4f) % t_background_day.width) * GetFrameTime();
    fg_scroll += ((int)(SCROLL_SPEED) % t_background_day.width) * GetFrameTime();

    bird_velocity += BIRD_GRAVITY * GetFrameTime();
    p_bird->position.y += bird_velocity;
    if (p_bird->position.y < t_bird.height * -1.5f) p_bird->position.y = t_bird.height * -1.5f;

    for(int i=0; i<PIPE_TOTAL; i++)
    {
      p_pipe[i]->position.x -= SCROLL_SPEED * GetFrameTime();

      if (p_pipe[i]->position.x < -t_pipe.width)
      {
        int tempY = GetRandomValue(150, win_screen.y - t_foreground.height - 40);
        p_pipe[i]->position = (Vector2){*p_lastPosX + PIPE_SPACE, tempY};
        p_lastPosX = &p_pipe[i*2]->position.x; // track last x position
      }
    }

    // TODO: calculate x,y hitbox (actually do i need that? the texture size already perfectly cropped)
    for(int i=0; i<PIPE_TOTAL; i++)
    {
      Rectangle rect_pipe_top = (Rectangle){
        p_pipe[i]->position.x, p_pipe[i]->position.y - t_pipe.height - PIPE_GAP,
        p_pipe[i]->hitbox.width, p_pipe[i]->hitbox.height};

      Rectangle rect_pipe_bot = (Rectangle){
        p_pipe[i]->position.x, p_pipe[i]->position.y + PIPE_GAP,
        p_pipe[i]->hitbox.width, p_pipe[i]->hitbox.height};

      Rectangle rect_bird = (Rectangle){
        p_bird->position.x, p_bird->position.y,
        p_bird->hitbox.width, p_bird->hitbox.height};

      if (CheckCollisionRecs(rect_bird, rect_pipe_top)) b_gameover = true;
      if (CheckCollisionRecs(rect_bird, rect_pipe_bot)) b_gameover = true;
    }

    if (p_bird->position.y + t_bird.height > win_screen.y - t_foreground.height) b_gameover = true;
  }
}
void Render()
{
  DrawTextureRec(
    t_background_day,
    (Rectangle){bg_scroll, 0.0f, t_background_day.width, t_background_day.height}, 
    (Vector2){0.0f, 0.0f}, WHITE);

  DrawTextureEx(t_bird, p_bird->position, 0.0f, 1.0f, WHITE);

  for(int i=0; i<PIPE_TOTAL; i++)
  {
    DrawTextureRec(
      t_pipe,
      (Rectangle){0.0f, 0.0f, t_pipe.width, -t_pipe.height},
      (Vector2){p_pipe[i]->position.x, p_pipe[i]->position.y - t_pipe.height - PIPE_GAP},
      WHITE);

    DrawTextureRec(
      t_pipe,
      (Rectangle){0.0f, 0.0f, t_pipe.width, t_pipe.height},
      (Vector2){p_pipe[i]->position.x, p_pipe[i]->position.y + PIPE_GAP},
      WHITE);
  }

  DrawTextureRec(
    t_foreground,
    (Rectangle){fg_scroll, 0.0f, t_foreground.width, t_foreground.height},
    (Vector2){0.0f, win_screen.y - t_foreground.height},
    WHITE);

  if (b_gameover)
  {
    DrawRectangle(0, 0, win_screen.x, win_screen.y, (Color){0, 0, 0, 125});
    DrawText("GAME OVER\nPress [R] to restart", 30, 80, 22, WHITE);
  }
}

void RenderDebug()
{
  DrawRectangleLinesEx(
    (Rectangle){
      p_bird->position.x, p_bird->position.y,
      p_bird->hitbox.width, p_bird->hitbox.height},
    3, RED);

  for(int i=0; i<PIPE_TOTAL; i++)
  {
    DrawRectangleLinesEx(
      (Rectangle){
        p_pipe[i]->position.x, p_pipe[i]->position.y - t_pipe.height - PIPE_GAP,
        p_pipe[i]->hitbox.width, p_pipe[i]->hitbox.height},
      3, BLUE);

    DrawRectangleLinesEx(
      (Rectangle){
        p_pipe[i]->position.x, p_pipe[i]->position.y + PIPE_GAP,
        p_pipe[i]->hitbox.width, p_pipe[i]->hitbox.height},
      3, BLUE);

    DrawText(
      TextFormat("pos: %i\nvel: %1f", (int)p_bird->position.y, bird_velocity),
      p_bird->position.x, p_bird->position.y + t_bird.height, 12, BLACK);

    DrawFPS(0, 0);
  }
}

void MainLoop()
{
  UpdateInput();
  UpdateLogic();

  BeginTextureMode(rt_buffer);
  {
    Render();
    if (b_debugDraw && !b_gameover) RenderDebug();
  };
  EndTextureMode();

  // #############

  BeginDrawing();
  {
    ClearBackground(BLACK); // letterbox bg color

    float scale = fmin(
      (float)GetScreenWidth()/rt_buffer.texture.width,
      (float)GetScreenHeight()/rt_buffer.texture.height);

    DrawTexturePro(
      rt_buffer.texture,
      (Rectangle){0.0f, 0.0f, rt_buffer.texture.width, -rt_buffer.texture.height},
      (Rectangle){
        (GetScreenWidth() - (rt_buffer.texture.width*scale))*0.5f,
        (GetScreenHeight() - (rt_buffer.texture.height*scale))*0.5f,
        rt_buffer.texture.width*scale, rt_buffer.texture.height*scale
      }, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
  }
  EndDrawing();
}
