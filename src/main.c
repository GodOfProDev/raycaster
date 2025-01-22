#include <SDL2/SDL.h>

#include <stdbool.h>
#include <math.h>

#include "util.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAP_WIDTH   24
#define MAP_HEIGHT  24

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;

	bool is_running;
} GameState;

typedef struct {
	float x;
	float y;
	float dir_x;
	float dir_y;
	float speed;
} Player;

typedef struct {
	float x;
	float y;
} CameraPlane;

GameState game_state;
Player player;
CameraPlane camera_plane;

int world_map[MAP_WIDTH][MAP_HEIGHT] =
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

void init_game() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		ERROR_EXIT("Could not init SDL: %s\n", SDL_GetError());
	}

	game_state.window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!game_state.window) {
		ERROR_EXIT("Could not create Window: %s\n", SDL_GetError());
	}

	game_state.renderer = SDL_CreateRenderer(game_state.window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!game_state.renderer) {
		ERROR_EXIT("Could not create Renderer: %s\n", SDL_GetError());
	}

	camera_plane.x = 0;
	camera_plane.y = 0.66;

	player.x = 22;
	player.y = 12;
	player.dir_x = -1;
	player.dir_y = 0;
	player.speed = 1000;

	game_state.is_running = true;
}

void process_events() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			game_state.is_running = false;
			break;
		}
	}
}

void shutdown_game() {
	SDL_DestroyWindow(game_state.window);
	game_state.window = NULL;
	SDL_DestroyRenderer(game_state.renderer);
	game_state.renderer = NULL;

	SDL_Quit();
}

Uint8* inkeys;

bool keyDown(SDL_Keycode key)
{
	return (inkeys[SDL_GetScancodeFromKey(key)] != 0);
}

int main(int argc, char* argv[]) {
	init_game();

	double time = 0;
	double oldTime = 0;

	while (game_state.is_running) {
		process_events();

		for (int x = 0; x < SCREEN_WIDTH; x++) {
			//calculate ray position and direction
			double cameraX = 2 * x / (double)(SCREEN_WIDTH) - 1; //x-coordinate in camera space
			double rayDirX = player.dir_x + camera_plane.x * cameraX;
			double rayDirY = player.dir_y + camera_plane.y * cameraX;

			//which box of the map we're in
			int mapX = (int)player.x;
			int mapY = (int)player.y;

			//length of ray from current position to next x or y-side
			double sideDistX;
			double sideDistY;

			//length of ray from one x or y-side to next x or y-side
			double deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
			double deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
			double perpWallDist;

			//what direction to step in x or y-direction (either +1 or -1)
			int stepX;
			int stepY;

			int hit = 0; //was there a wall hit?
			int side; //was a NS or a EW wall hit?

			//calculate step and initial sideDist
			if (rayDirX < 0)
			{
				stepX = -1;
				sideDistX = (player.x - mapX) * deltaDistX;
			}
			else
			{
				stepX = 1;
				sideDistX = (mapX + 1.0 - player.x) * deltaDistX;
			}
			if (rayDirY < 0)
			{
				stepY = -1;
				sideDistY = (player.y - mapY) * deltaDistY;
			}
			else
			{
				stepY = 1;
				sideDistY = (mapY + 1.0 - player.y) * deltaDistY;
			}

			//perform DDA
			while (hit == 0)
			{
				//jump to next map square, either in x-direction, or in y-direction
				if (sideDistX < sideDistY)
				{
					sideDistX += deltaDistX;
					mapX += stepX;
					side = 0;
				}
				else
				{
					sideDistY += deltaDistY;
					mapY += stepY;
					side = 1;
				}
				//Check if ray has hit a wall
				if (world_map[mapX][mapY] > 0) hit = 1;
			}

			//Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
			if (side == 0) 
				perpWallDist = (sideDistX - deltaDistX);
			else         
				perpWallDist = (sideDistY - deltaDistY);

			//Calculate height of line to draw on screen
			int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

			//calculate lowest and highest pixel to fill in current stripe
			int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
			if (drawStart < 0)
				drawStart = 0;
			int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
			if (drawEnd >= SCREEN_HEIGHT)
				drawEnd = SCREEN_HEIGHT - 1;

			//choose wall color
			int r = 0;
			int g = 0;
			int b = 0;

			switch (world_map[mapX][mapY])
			{
			case 1: { r = 255;  break; } //red
			case 2: { g = 255;  break;} //green
			case 3: { b = 255;   break; } //blue
			case 4: { r = 255; g = 255; b = 250;  break; } //white
			default: { r = 255; g = 255; break; } //yellow
			}

			//give x and y sides different brightness
			if (side == 1) { 
				r = r / 2;
				g = g / 2;
				b = b / 2;
			} // Dim RGB channels

			SDL_SetRenderDrawColor(game_state.renderer, 
				r, g, b, 255);

			SDL_RenderDrawLine(game_state.renderer, x, drawStart, x, drawEnd);
		}

		oldTime = time;
		time = SDL_GetTicks();
		double frameTime = (time - oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds

		SDL_RenderPresent(game_state.renderer);

		SDL_SetRenderDrawColor(game_state.renderer, 0, 0, 0, 0xFF);
		SDL_RenderClear(game_state.renderer);

		double moveSpeed = frameTime * 5.0; //the constant value is in squares/second
		double rotSpeed = frameTime * 3.0; //the constant value is in radians/second

		inkeys = SDL_GetKeyboardState(NULL);
		//move forward if no wall in front of you
		if (keyDown(SDLK_UP))
		{
			if (world_map[(int)(player.x + player.dir_x * moveSpeed)][(int)(player.y)] == false) player.x += player.dir_x * moveSpeed;
			if (world_map[(int)(player.x)][(int)(player.y + player.dir_y * moveSpeed)] == false) player.y += player.dir_y * moveSpeed;
		}
		//move backwards if no wall behind you
		if (keyDown(SDLK_DOWN))
		{
			if (world_map[(int)(player.x - player.dir_x * moveSpeed)][(int)(player.y)] == false) player.x -= player.dir_x * moveSpeed;
			if (world_map[(int)(player.x)][(int)(player.y - player.dir_y * moveSpeed)] == false) player.y -= player.dir_y * moveSpeed;
		}
		//rotate to the right
		if (keyDown(SDLK_RIGHT))
		{
			//both camera direction and camera plane must be rotated
			double oldDirX = player.dir_x;
			player.dir_x = player.dir_x * cos(-rotSpeed) - player.dir_y * sin(-rotSpeed);
			player.dir_y = oldDirX * sin(-rotSpeed) + player.dir_y * cos(-rotSpeed);
			double oldPlaneX = camera_plane.x;
			camera_plane.x = camera_plane.x * cos(-rotSpeed) - camera_plane.y * sin(-rotSpeed);
			camera_plane.y = oldPlaneX * sin(-rotSpeed) + camera_plane.y * cos(-rotSpeed);
		}
		//rotate to the left
		if (keyDown(SDLK_LEFT))
		{
			//both camera direction and camera plane must be rotated
			double oldDirX = player.dir_x;
			player.dir_x = player.dir_x * cos(rotSpeed) - player.dir_y * sin(rotSpeed);
			player.dir_y = oldDirX * sin(rotSpeed) + player.dir_y * cos(rotSpeed);
			double oldPlaneX = camera_plane.x;
			camera_plane.x = camera_plane.x * cos(rotSpeed) - camera_plane.y * sin(rotSpeed);
			camera_plane.y = oldPlaneX * sin(rotSpeed) + camera_plane.y * cos(rotSpeed);
		}
	}

	shutdown_game();
	return 0;
}