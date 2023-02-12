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
#include<fstream>
using namespace std;

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif
#include "networking.h"

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------

static const int wallPixel = 25;
static const int mapWidth = 40;
static const int mapHeight = 24;

static const int screenWidth = wallPixel*mapWidth;
static const int screenHeight = wallPixel*mapHeight;


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

Shader shader;
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "classic game: space invaders");

    e.lightShader = LoadShader("resources/shaders/base.vs","resources/shaders/base.fs");
    for(int i = 0;i<100;i++){
        e.lightShaderFocusLocs.push_back(GetShaderLocation(e.lightShader, TextFormat("focuspoints[%i]\0",i)));
    }
    e.light_mask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    InitGame();
    socket = Websocket(&e);
    socket.ConnectServer();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif
    UnloadGame();
    CloseWindow();
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
    score = 0;
    alpha = 0;

    ifstream fin("resources/charmap.txt");
    for (int i = 0; i < mapHeight; i++) {
        for (int j = 0; j < mapWidth; j++) {
            char c;
            fin >> c;
            if (c != '.') {
                Wall wall;
                wall.rec.width = wallPixel;
                wall.rec.height = wallPixel;
                wall.rec.x = wallPixel*j;
                wall.rec.y = wallPixel*i;
                wall.active = true;
                wall.color = DARKGRAY;
                wall.mirror = c == '*';
                if (c == '#' || c == '*') {
                    e.walls.insert(wall);
                }
                else if (c == '@'){
                    e.backgroundWalls.insert(wall);
                }
            }
        }
    }
    fin.close();

	adding_mirror.active = false;
}

int EPS = 5;
float angle = 0;

// Update game (one frame)
void UpdateGame(void)
{
    socket.SendPosition();
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        if(!adding_mirror.active){
            // Make mirror visible
            adding_mirror.active = true;
        }
        else{
            e.mirrors.push_back(adding_mirror);
            adding_mirror.active = false;
            angle = 0;
        }
	}
    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && adding_mirror.active){
        adding_mirror.active = false;
        angle = 0;
    }
    if(adding_mirror.active){
        // Move and rotate
        Point p = Point(GetMousePosition());
        if ((p-e.player.midpoint()).dist() > 150){
            p = e.player.midpoint() + (p-e.player.midpoint()).unit() * 150;
        }
        adding_mirror.seg.p1 = p;
        adding_mirror.seg.p2 = p;
        adding_mirror.seg.p1.x -= 30;
        adding_mirror.seg.p2.x += 30;
        angle += GetMouseWheelMove() / 20.0;
        adding_mirror.seg.rotate(angle);
    }

    if (!gameOver && !victory)
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

        for(LightFrustrum light : e.lightFrustra){
            if(e.player.inside(light)){
                e.player.damage();
            }
        }

        if(!e.player.alive()){
            gameOver = true;
        }
        if(!e.opponent.alive()){
            victory = true;
        }

		// Edges behaviour
		if (e.player.rec.x <= 0) e.player.rec.x = 0;
		if (e.player.rec.x + e.player.rec.width >= screenWidth) e.player.rec.x = screenWidth - e.player.rec.width;
		if (e.player.rec.y <= 0) e.player.rec.y = 0;
		if (e.player.rec.y + e.player.rec.height >= screenHeight) e.player.rec.y = screenHeight - e.player.rec.height;

        
        
        //if(e.lightFrustra.size()==0){
            // Mirror mirror;
            // mirror.active = true;
            // mirror.seg = Segment(Point(200,400), Point(250,400));
            // e.mirrors.push_back(mirror);
            // mirror.seg = Segment(Point(450,450), Point(450,500));
            // e.mirrors.push_back(mirror);

            vector<ObstacleForSim> obstacles;
            obstacles.push_back(ObstacleForSim(Segment(Point(0,0),Point(0,screenHeight)),wall));
            obstacles.push_back(ObstacleForSim(Segment(Point(screenWidth,screenHeight),Point(0,screenHeight)),wall));
            obstacles.push_back(ObstacleForSim(Segment(Point(screenWidth,screenHeight),Point(screenWidth,0)),wall));
            obstacles.push_back(ObstacleForSim(Segment(Point(0,0),Point(screenWidth,0)),wall));

            for(Mirror mirror : e.mirrors){
                obstacles.push_back(ObstacleForSim(mirror.seg,double_mirror));
            }

            for(Wall w : e.walls){
                ObstacleType type = w.mirror ? double_mirror : wall;
                obstacles.push_back(ObstacleForSim(Segment(Point(w.rec.x, w.rec.y), Point(w.rec.x + w.rec.width, w.rec.y)), type));
                obstacles.push_back(ObstacleForSim(Segment(Point(w.rec.x + w.rec.width, w.rec.y), Point(w.rec.x + w.rec.width, w.rec.y + w.rec.height)), type));
                obstacles.push_back(ObstacleForSim(Segment(Point(w.rec.x, w.rec.y + w.rec.height), Point(w.rec.x + w.rec.width, w.rec.y + w.rec.height)), type));
                obstacles.push_back(ObstacleForSim(Segment(Point(w.rec.x, w.rec.y), Point(w.rec.x, w.rec.y + w.rec.height)), type));
            }
            obstacles.push_back(ObstacleForSim(Segment(Point(e.opponent.rec.x, e.opponent.rec.y), Point(e.opponent.rec.x + e.opponent.rec.width, e.opponent.rec.y)), wall));
            obstacles.push_back(ObstacleForSim(Segment(Point(e.opponent.rec.x + e.opponent.rec.width, e.opponent.rec.y), Point(e.opponent.rec.x + e.opponent.rec.width, e.opponent.rec.y + e.opponent.rec.height)), wall));
            obstacles.push_back(ObstacleForSim(Segment(Point(e.opponent.rec.x, e.opponent.rec.y + e.opponent.rec.height), Point(e.opponent.rec.x + e.opponent.rec.width, e.opponent.rec.y + e.opponent.rec.height)), wall));
            obstacles.push_back(ObstacleForSim(Segment(Point(e.opponent.rec.x, e.opponent.rec.y), Point(e.opponent.rec.x, e.opponent.rec.y + e.opponent.rec.height)), wall));

           vector<LightFrustrumForSim> startingLights = {
                   LightFrustrumForSim(Point(200,100),Segment(Point(300,200),Point(150,200))),
                   LightFrustrumForSim(Point(800,500),Segment(Point(750,450),Point(850,450)))
            };
            // obstacles.push_back(ObstacleForSim(adding_mirror.seg,double_mirror));
            e.lightFrustra.clear();
            for (LightFrustrumForSim light : startingLights) {
                auto newLights =run_light_simulation(obstacles, light);
                e.lightFrustra.reserve(e.lightFrustra.size() + newLights.size());
                e.lightFrustra.insert(e.lightFrustra.end(),newLights.begin(),newLights.end());
            }

            //e.lightFrustra.push_back(LightFrustrum(Point(200,100),Segment(Point(300,200),Point(150,200)),Segment(Point(300,200),Point(150,200))));
        //}
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

			if(adding_mirror.active) adding_mirror.draw();
            
            DrawRectangleLinesEx({20, 20, screenWidth / 4, 20}, 5, {0, 0,0,120});
            DrawRectangle(20, 20, screenWidth / 4 * e.player.getHealth(), 20, {255, 0,0,120});

            DrawRectangleLinesEx({screenWidth * 3 / 4 - 20, 20, screenWidth / 4, 20}, 5, {0, 0,0,120});
            DrawRectangle(screenWidth - 20 - screenWidth * e.opponent.getHealth() / 4, 20, screenWidth / 4 * e.opponent.getHealth(), 20, {255, 0,0,120});

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

