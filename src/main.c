#include <stdio.h>
#include <raylib.h>
#include <math.h>

// --- CONSTANTS ------------>

#define WIDTH 900
#define HEIGHT 600

#define BRANCH_COLOR BLACK
#define INITIAL_THICKNESS 20

// --- PROTOTYPES ------------>

void DrawBranch(float, float, float, float, float);

// --- ENTRY POINT ------------>

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "Fractal Tree");

	SetTargetFPS(1);

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(WHITE);
		DrawBranch(WIDTH / 2, HEIGHT - 20, 300, 0, INITIAL_THICKNESS);
		EndDrawing();
	}

	CloseWindow();

	return 0;
}

// --- IMPLEMENTATIONS ------------>

void DrawBranch(float x, float y, float length, float angle, float thickness)
{
	Vector2 start = {x, y};
	Vector2 end = {x, y - length};

	DrawLineEx(start, end, thickness, BRANCH_COLOR);
}