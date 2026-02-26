#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "Fractal Tree");

	SetTargetFPS(1);

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(WHITE);
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
