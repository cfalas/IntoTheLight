/*******************************************************************************************
*
*   raylib - classic game: space invaders
*
*   Sample game developed by Ian Eito, Albert Martos and Ramon Santamaria
*
*   This game has been created using raylib v1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include<vector>
using namespace std;

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif
#include "networking.h"
#include "entities.h"

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause =  false;
static int score = 0;
static bool victory = false;

static Player player;
static vector<Wall> walls;

static float alpha = 0.0f;

static int numWalls = 10;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "classic game: space invaders");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------
Mirror adding_mirror;

// Initialize game variables
void InitGame(void)
{
    // Initialize game variables
    pause = false;
    gameOver = false;
    victory = false;
	numWalls = 10;
    walls.assign(10, Wall());
    score = 0;
    alpha = 0;

    // Initialize player
    player.rec.x =  20;
    player.rec.y = 50;
    player.rec.width = 20;
    player.rec.height = 20;
    player.maxspeed = 5;
    player.color = BLACK;

    for (int i = 0; i < numWalls; i++)
    {
        walls[i].rec.width = 10;
        walls[i].rec.height = 10;
        walls[i].rec.x = GetRandomValue(0, screenWidth - walls[i].rec.width);
        walls[i].rec.y = GetRandomValue(0, screenHeight - walls[i].rec.height);
        walls[i].active = true;
        walls[i].color = GRAY;
    }
	adding_mirror.color = RED;
	adding_mirror.active = true;
	adding_mirror.rec.width = 60;
	adding_mirror.rec.height = 2;
}

int EPS = 5;
Vector2 mousePosition;
bool adding = false;
vector<Mirror> mirrors;

// Update game (one frame)
void UpdateGame(void)
{
    //ws->UpdatePosition(player.rec.x, player.rec.y);
	if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
		if(!adding){
			// Place mirror
			adding = true;
			mousePosition = GetMousePosition();
			adding_mirror.rec.x = mousePosition.x;
			adding_mirror.rec.y = mousePosition.y;
		}
		else{
			// Rotate mirror
			adding_mirror.angle += GetMouseDelta().x;
		}
	}
	else if(adding){
		adding = false;
		mirrors.push_back(adding_mirror);
		adding_mirror.angle = 0;
	}
    if (!gameOver)
    {

		// Player movement
        player.setSpeed();
        player.move();

		// Player collision with walls
        for(Wall wall : walls)
		{
            if(player.collides(wall)){
                player.fixCollision(wall);
            }
		}

		// Edges behaviour
		if (player.rec.x <= 0) player.rec.x = 0;
		if (player.rec.x + player.rec.width >= screenWidth) player.rec.x = screenWidth - player.rec.width;
		if (player.rec.y <= 0) player.rec.y = 0;
		if (player.rec.y + player.rec.height >= screenHeight) player.rec.y = screenHeight - player.rec.height;

	}
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);

        if (!gameOver)
        {
            DrawRectangleRec(player.rec, player.color);

            for (int i = 0; i < numWalls; i++) {
                if (walls[i].active) DrawRectangleRec(walls[i].rec, walls[i].color);
            }

			for(auto v : mirrors){
				if(v.active) DrawRectanglePro(v.rec, {v.rec.width/2, v.rec.height/2}, v.angle, v.color);
            }
			if(adding) DrawRectanglePro(adding_mirror.rec, {adding_mirror.rec.width/2, adding_mirror.rec.height/2}, adding_mirror.angle, adding_mirror.color);

            DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY);

            if (victory) DrawText("YOU WIN", screenWidth/2 - MeasureText("YOU WIN", 40)/2, screenHeight/2 - 40, 40, BLACK);

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}

