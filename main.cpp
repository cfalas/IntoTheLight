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
#include "lightsim.h"
#include "entities.h"
#include<vector>
using namespace std;

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif
#include "networking.h"

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause =  false;
static int score = 0;
static bool victory = false;

static Environment e;
static float alpha = 0.0f;

Websocket socket;

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
    socket = Websocket(&e);
    socket.ConnectServer();

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
    int numWalls = 10;
    e.walls.assign(numWalls, Wall());
    score = 0;
    alpha = 0;


    for (int i = 0; i < numWalls; i++)
    {
        e.walls[i].rec.width = 10;
        e.walls[i].rec.height = 10;
        e.walls[i].rec.x = GetRandomValue(0, screenWidth - e.walls[i].rec.width);
        e.walls[i].rec.y = GetRandomValue(0, screenHeight - e.walls[i].rec.height);
        e.walls[i].active = true;
        e.walls[i].color = GRAY;
    }
	adding_mirror.active = true;
	adding_mirror.length = 60;
}

int EPS = 5;
bool adding = false;

// Update game (one frame)
void UpdateGame(void)
{
    socket.SendPosition();
	if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
		if(!adding){
			// Place mirror
			adding = true;
            adding_mirror.pos = GetMousePosition();
		}
		else{
			// Rotate mirror
			adding_mirror.angle += GetMouseDelta().x;
		}
	}
	else if(adding){
		adding = false;
		e.mirrors.push_back(adding_mirror);
		adding_mirror.angle = 0;
	}
    if (!gameOver)
    {

		// Player movement
        e.player.setSpeed();
        e.player.move();

		// Player collision with walls
        for(Wall wall : e.walls)
		{
            if(e.player.collides(wall)){
                e.player.fixCollision(wall);
            }
		}

		// Edges behaviour
		if (e.player.rec.x <= 0) e.player.rec.x = 0;
		if (e.player.rec.x + e.player.rec.width >= screenWidth) e.player.rec.x = screenWidth - e.player.rec.width;
		if (e.player.rec.y <= 0) e.player.rec.y = 0;
		if (e.player.rec.y + e.player.rec.height >= screenHeight) e.player.rec.y = screenHeight - e.player.rec.height;


        static const int screenWidth = 800;
        static const int screenHeight = 450;
        LightFrustrumForSim lightForSim(Point(200,100),Segment(Point(300,200),Point(150,200)));
        vector<ObstacleForSim> obstacles;
        obstacles.push_back(ObstacleForSim(Segment(Point(0,0),Point(0,screenHeight)),wall));
        obstacles.push_back(ObstacleForSim(Segment(Point(screenWidth,screenHeight),Point(0,screenHeight)),wall));
        obstacles.push_back(ObstacleForSim(Segment(Point(screenWidth,screenHeight),Point(screenWidth,0)),wall));
        obstacles.push_back(ObstacleForSim(Segment(Point(0,0),Point(screenWidth,0)),double_mirror));

        
        for(Mirror mirror : e.mirrors){
            obstacles.push_back(ObstacleForSim(Segment(Point(mirror.pos.x,mirror.pos.y),Point(mirror.pos.x+10,mirror.pos.y+10)),double_mirror));
        }

        e.lightFrustra = run_light_simulation(obstacles, lightForSim);

	}
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);

        if (!gameOver)
        {
            e.draw();

			if(adding) adding_mirror.draw();

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

