#include <raylib.h>
#include <math.h>

// --- CONSTANTS ------------>

#define WIDTH 900
#define HEIGHT 600

#define RAD(angle) ((angle) * DEG2RAD)

#define BRANCH_COLOR RAYWHITE
#define INITIAL_THICKNESS 15
#define INITIAL_LENGTH 150

#define INITIAL_SPREAD_ANGLE RAD(20)
#define SPREAD_ANGLE_STEP RAD(5)
#define SPREAD_ANGLE_MIN RAD(1)
#define SPREAD_ANGLE_MAX RAD(90)

#define INITIAL_BRANCH_COUNT 2
#define BRANCH_COUNT_MIN 1
#define BRANCH_COUNT_MAX 6

#define LENGTH_REDUCTION_RATIO 0.75f
#define THICKNESS_REDUCTION_RATIO 0.75f
#define LENGTH_LIMIT (INITIAL_LENGTH * 0.1f)

// --- PROTOTYPES ------------>

void DrawBranch(float x, float y, float length, float angle, float thickness, float spreadAngle, int branchCount, Color color);
void RedrawTree(RenderTexture2D target, float spreadAngle, int branchCount);

// --- ENTRY POINT ------------>

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "Fractal Tree");
	SetTargetFPS(60);

	float spreadAngle = INITIAL_SPREAD_ANGLE;
	int branchCount = INITIAL_BRANCH_COUNT;

	RenderTexture2D target = LoadRenderTexture(WIDTH, HEIGHT);
	RedrawTree(target, spreadAngle, branchCount);

	while (!WindowShouldClose())
	{
		// --- INPUT ------------>

		bool changed = false;

		// Spread angle — arrow keys
		if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_RIGHT))
		{
			spreadAngle += SPREAD_ANGLE_STEP;
			if (spreadAngle > SPREAD_ANGLE_MAX)
				spreadAngle = SPREAD_ANGLE_MAX;
			changed = true;
		}
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_LEFT))
		{
			spreadAngle -= SPREAD_ANGLE_STEP;
			if (spreadAngle < SPREAD_ANGLE_MIN)
				spreadAngle = SPREAD_ANGLE_MIN;
			changed = true;
		}

		// Branch count — W/S keys
		if (IsKeyPressed(KEY_W))
		{
			if (branchCount < BRANCH_COUNT_MAX)
				branchCount++;
			changed = true;
		}
		if (IsKeyPressed(KEY_S))
		{
			if (branchCount > BRANCH_COUNT_MIN)
				branchCount--;
			changed = true;
		}

		// Reset — R
		if (IsKeyPressed(KEY_R))
		{
			spreadAngle = INITIAL_SPREAD_ANGLE;
			branchCount = INITIAL_BRANCH_COUNT;
			changed = true;
		}

		if (changed)
			RedrawTree(target, spreadAngle, branchCount);

		// --- DRAW ------------>

		BeginDrawing();
		DrawTextureRec(
			target.texture,
			(Rectangle){0, 0, WIDTH, -HEIGHT},
			(Vector2){0, 0},
			WHITE);
		DrawText(TextFormat("Spread: %.0f deg (UP/DOWN)", spreadAngle / DEG2RAD),
				 10, 10, 18, GRAY);
		DrawText(TextFormat("Branches: %d (W/S)  R to reset", branchCount),
				 10, 32, 18, GRAY);
		EndDrawing();
	}

	UnloadRenderTexture(target);
	CloseWindow();

	return 0;
}

// --- IMPLEMENTATIONS ------------>

void RedrawTree(RenderTexture2D target, float spreadAngle, int branchCount)
{
	BeginTextureMode(target);
	ClearBackground(BLACK);
	DrawBranch(WIDTH / 2, HEIGHT - 20, INITIAL_LENGTH, 0.0f, INITIAL_THICKNESS, spreadAngle, branchCount, BRANCH_COLOR);
	EndTextureMode();
}

/*
	@param x Starting x position
	@param y Starting y position
	@param length Length of the branch
	@param angle Angle of the branch from the vertical
	@param thickness Thickness of the branch
	@param spreadAngle Half-arc within which child branches are distributed
	@param branchCount Number of child branches to spawn
	@param color Color of the branch
*/
void DrawBranch(float x, float y, float length, float angle, float thickness, float spreadAngle, int branchCount, Color color)
{
	float x_end = x + sinf(angle) * length;
	float y_end = y - cosf(angle) * length;

	DrawLineEx((Vector2){x, y}, (Vector2){x_end, y_end}, thickness, color);

	float new_length = length * LENGTH_REDUCTION_RATIO;
	float new_thickness = thickness * THICKNESS_REDUCTION_RATIO;

	if (new_length < LENGTH_LIMIT || new_thickness < 1.0f)
		return;

	for (int i = 0; i < branchCount; i++)
	{
		float child_angle;

		if (branchCount == 1)
			child_angle = angle; // straight up, no spread
		else
			child_angle = angle + spreadAngle * (i - (branchCount - 1) / 2.0f) * (2.0f / (branchCount - 1));

		DrawBranch(x_end, y_end, new_length, child_angle, new_thickness, spreadAngle, branchCount, color);
	}
}