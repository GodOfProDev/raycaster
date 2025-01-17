#include <SDL2/SDL.h>

#include <stdbool.h>
#include <math.h>

#include "util.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define MAP_WIDTH   20
#define MAP_HEIGHT  20
#define CELL_WIDTH  32
#define CELL_HEIGHT 32

typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;

	int mouse_x;
	int mouse_y;

	double delta_time;

	bool is_left_click;
	

	bool is_running;
} GameState;

typedef struct {
	float x;
	float y;
	float speed;
} Player;

GameState game_state;
Player player;

int cell_grid[MAP_WIDTH * MAP_HEIGHT];

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

	player.x = SCREEN_WIDTH / 2.0f;
	player.y = SCREEN_HEIGHT / 2.0f;
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
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
			case SDLK_UP:
				player.y -= player.speed * game_state.delta_time;
				break;
			case SDLK_DOWN:
				player.y += player.speed * game_state.delta_time;
				break;
			case SDLK_LEFT:
				player.x -= player.speed * game_state.delta_time;
				break;
			case SDLK_RIGHT:
				player.x += player.speed * game_state.delta_time;
				break;
			}
		}
	}
}

void update() {
	int mouse_x, mouse_y;
	Uint32 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
	game_state.mouse_x = mouse_x;
	game_state.mouse_y = mouse_y;
	game_state.is_left_click = false;
	if (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
		int cell_x = mouse_x / CELL_WIDTH;
		int cell_y = mouse_y / CELL_HEIGHT;
		cell_grid[cell_y * MAP_WIDTH + cell_x] = 1;
	}
	if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		game_state.is_left_click = true;
	}
}

void DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
	// Validate input parameters
	if (renderer == NULL || radius < 0) {
		return;
	}

	for (int y = -radius; y <= radius; y++) {
		for (int x = -radius; x <= radius; x++) {
			if (x * x + y * y <= radius * radius) {
				SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
			}
		}
	}
}

void DrawDashedLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
	// Validate input parameters
	if (renderer == NULL) {
		return;
	}

	int dashLength = 10;
	int gapLength = 5;

	// Calculate the line length and angle
	double dx = x2 - x1;
	double dy = y2 - y1;
	double length = sqrt(dx * dx + dy * dy);
	double angle = atan2(dy, dx);

	// Calculate the step sizes
	double stepX = (dashLength * cos(angle));
	double stepY = (dashLength * sin(angle));
	double gapStepX = (gapLength * cos(angle));
	double gapStepY = (gapLength * sin(angle));

	// Starting position
	double currentX = x1;
	double currentY = y1;
	double segmentLength = dashLength + gapLength;
	double remainingLength = length;
	bool drawing = true;  // Start with a dash

	while (remainingLength > 0) {
		if (drawing) {
			// Draw dash
			double endX = currentX + stepX;
			double endY = currentY + stepY;

			// Ensure we don't draw past the end point
			if (remainingLength < dashLength) {
				endX = x2;
				endY = y2;
			}

			SDL_RenderDrawLine(renderer,
				(int)round(currentX), (int)round(currentY),
				(int)round(endX), (int)round(endY));

			currentX = endX;
			currentY = endY;
			remainingLength -= dashLength;
		}
		else {
			// Skip gap
			currentX += gapStepX;
			currentY += gapStepY;
			remainingLength -= gapLength;
		}

		drawing = !drawing;  // Toggle between dash and gap
	}
}

void render() {
	SDL_Renderer* rd = game_state.renderer;
	SDL_SetRenderDrawColor(rd, 0, 0, 0, 255);
	SDL_RenderClear(rd);

	// White Border
	SDL_SetRenderDrawColor(rd, 255, 255, 255, 255);
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			SDL_Rect grid_cell = {
				.x = x * CELL_WIDTH,
				.y = y * CELL_HEIGHT,
				.w = CELL_WIDTH,
				.h = CELL_HEIGHT,
			};

			int cell = cell_grid[y * MAP_WIDTH + x];
			if (cell) {
				SDL_SetRenderDrawColor(rd, 0, 177, 0, 255);
				SDL_RenderFillRect(rd, &grid_cell);
				SDL_SetRenderDrawColor(rd, 255, 255, 255, 255); // White Border
			}

			SDL_RenderDrawRect(rd, &grid_cell);
		}
	}

	SDL_SetRenderDrawColor(rd, 255, 0, 0, 255);
	DrawFilledCircle(rd, player.x, player.y, 4);

	SDL_SetRenderDrawColor(rd, 0, 255, 0, 255);
	DrawFilledCircle(rd, game_state.mouse_x, game_state.mouse_y, 4);

	float player_grid_x = (player.x / CELL_WIDTH);
	float player_grid_y = (player.y / CELL_HEIGHT);

	float ray_start_x = player_grid_x;
	float ray_start_y = player_grid_y;
	float ray_dir_x = (game_state.mouse_x - player.x);
	float ray_dir_y = (game_state.mouse_y - player.y);

	float length = sqrt(ray_dir_x * ray_dir_x + ray_dir_y * ray_dir_y);

	// Avoid division by zero
	if (length != 0) {
		// Normalize by dividing each component by the length
		ray_dir_x = ray_dir_x / length;
		ray_dir_y = ray_dir_y / length;
	}

	float ray_unit_step_size_x = (ray_dir_x == 0) ? 1e30 : sqrt(1 + (ray_dir_y * ray_dir_y) / (ray_dir_x * ray_dir_x));
	float ray_unit_step_size_y = (ray_dir_x == 0) ? 1e30 : sqrt(1 + (ray_dir_x * ray_dir_x) / (ray_dir_y * ray_dir_y));
	int map_check_x = player_grid_x;
	int map_check_y = player_grid_y;

	float ray_length_x;
	float ray_length_y;

	int step_x = (ray_dir_x < 0) ? -1 : 1;
	int step_y = (ray_dir_y < 0) ? -1 : 1;

	ray_length_x = (ray_dir_x < 0) ?
		(ray_start_x - (float)(map_check_x)) * ray_unit_step_size_x :
		((float)(map_check_x + 1) - ray_start_x) * ray_unit_step_size_x;

	ray_length_y = (ray_dir_y < 0) ?
		(ray_start_y - (float)(map_check_y)) * ray_unit_step_size_y :
		((float)(map_check_y + 1) - ray_start_y) * ray_unit_step_size_y;

	bool tile_found = false;
	float max_distance = 100;
	float distance = 0.0f;

	while (!tile_found && distance < max_distance) {
		if (ray_length_x < ray_length_y) {
			map_check_x += step_x;
			distance = ray_length_x;
			ray_length_x += ray_unit_step_size_x;
		}
		else {
			map_check_y += step_y;
			distance = ray_length_y;
			ray_length_y += ray_unit_step_size_y;
		}

		if (map_check_x >= 0 && map_check_x < MAP_WIDTH && map_check_y >= 0 && map_check_y < MAP_HEIGHT) {
			if (cell_grid[map_check_y * MAP_WIDTH + map_check_x] == 1) {
				tile_found = true;
			}
		}
	}

	float intersection_x = (ray_start_x + ray_dir_x * distance) * CELL_WIDTH;
	float intersection_y = (ray_start_y + ray_dir_y * distance) * CELL_HEIGHT;

	if (game_state.is_left_click) {
		SDL_SetRenderDrawColor(rd, 255, 255, 255, 255);
		DrawDashedLine(rd, player.x, player.y, game_state.mouse_x, game_state.mouse_y);

		if (tile_found) {
			SDL_SetRenderDrawColor(rd, 255, 255, 0, 255);
			DrawFilledCircle(rd, intersection_x, intersection_y, 4);
		}
	}

	SDL_RenderPresent(rd);
}

void shutdown_game() {
	SDL_DestroyWindow(game_state.window);
	game_state.window = NULL;
	SDL_DestroyRenderer(game_state.renderer);
	game_state.renderer = NULL;

	SDL_Quit();
}

int main(int argc, char* argv[]) {
	init_game();

	Uint64 now = SDL_GetPerformanceCounter();
	Uint64 last = 0;
	double delta_time = 0;

	while (game_state.is_running) {
		last = now;
		now = SDL_GetPerformanceCounter();

		delta_time = ((now - last) / (double)SDL_GetPerformanceFrequency());
		game_state.delta_time = delta_time;

		process_events();
		update();
		render();
	}

	shutdown_game();
	return 0;
}