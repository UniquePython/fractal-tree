#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// --- CONSTANTS ------------>

#define WIDTH 900
#define HEIGHT 600

#define RAD(angle) ((angle) * DEG2RAD)
#define RAND_F() ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f)

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

#define ANGLE_JITTER RAD(8)
#define LENGTH_JITTER 0.15f

#define TRUNK_COLOR (Color){101, 67, 33, 255}
#define LEAF_COLOR (Color){34, 139, 34, 255}
#define MAX_DEPTH 12

#define WIND_BASE RAD(0.8f)
#define WIND_GUST_STRENGTH RAD(5.0f)
#define WIND_FREQ 0.5f
#define WIND_PHASE_OFFSET 0.4f
#define WIND_DEPTH_SCALE 0.25f

// Auto depth cap to keep performance reasonable at high branch counts
#define AUTO_DEPTH_CAP(bc) ((bc) <= 2 ? MAX_DEPTH : (bc) <= 3 ? 8 \
												: (bc) <= 4	  ? 6 \
															  : 5)

// --- PROTOTYPES ------------>

void DrawBranch(float x, float y, float length, float angle, float thickness, float spreadAngle, int branchCount, int depth, int maxDepth, float wind, float time);

// --- ENTRY POINT ------------>

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "Fractal Tree");
	SetTargetFPS(60);

	float spreadAngle = INITIAL_SPREAD_ANGLE;
	int branchCount = INITIAL_BRANCH_COUNT;

	// Store per-branch jitter offsets so they don't change every frame
	unsigned int seed = (unsigned int)time(NULL);
	srand(seed);

	while (!WindowShouldClose())
	{
		// --- INPUT ------------>

		bool reseed = false;

		if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_RIGHT))
		{
			spreadAngle += SPREAD_ANGLE_STEP;
			if (spreadAngle > SPREAD_ANGLE_MAX)
				spreadAngle = SPREAD_ANGLE_MAX;
		}
		if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_LEFT))
		{
			spreadAngle -= SPREAD_ANGLE_STEP;
			if (spreadAngle < SPREAD_ANGLE_MIN)
				spreadAngle = SPREAD_ANGLE_MIN;
		}
		if (IsKeyPressed(KEY_W))
		{
			if (branchCount < BRANCH_COUNT_MAX)
				branchCount++;
			reseed = true;
		}
		if (IsKeyPressed(KEY_S))
		{
			if (branchCount > BRANCH_COUNT_MIN)
				branchCount--;
			reseed = true;
		}
		if (IsKeyPressed(KEY_SPACE))
			reseed = true;
		if (IsKeyPressed(KEY_R))
		{
			spreadAngle = INITIAL_SPREAD_ANGLE;
			branchCount = INITIAL_BRANCH_COUNT;
			reseed = true;
		}

		if (reseed)
		{
			seed = (unsigned int)time(NULL);
			srand(seed);
		}

		// --- WIND ------------>

		float t = (float)GetTime();
		float gust = sinf(t * 0.2f) * sinf(t * 0.5f) * sinf(t * 1.1f); // many frequencies = irregular gusts
		float wind = WIND_BASE + gust * WIND_GUST_STRENGTH;

		// --- DRAW ------------>

		// Reset rand to same seed each frame so jitter is stable
		srand(seed);

		BeginDrawing();
		ClearBackground(BLACK);

		DrawBranch(
			WIDTH / 2, HEIGHT - 20,
			INITIAL_LENGTH, 0.0f, INITIAL_THICKNESS,
			spreadAngle, branchCount, 0,
			AUTO_DEPTH_CAP(branchCount),
			wind, t);

		DrawText(TextFormat("Spread: %.0f deg (UP/DOWN)", spreadAngle / DEG2RAD), 10, 10, 18, GRAY);
		DrawText(TextFormat("Branches: %d (W/S)", branchCount), 10, 32, 18, GRAY);
		DrawText("SPACE: new tree   R: reset", 10, 54, 18, GRAY);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}

// --- IMPLEMENTATIONS ------------>

void DrawBranch(float x, float y, float length, float angle, float thickness, float spreadAngle, int branchCount, int depth, int maxDepth, float wind, float t)
{
	float x_end = x + sinf(angle) * length;
	float y_end = y - cosf(angle) * length;

	float color_t = (float)depth / (float)MAX_DEPTH;
	if (color_t > 1.0f)
		color_t = 1.0f;
	Color branch_color = ColorLerp(TRUNK_COLOR, LEAF_COLOR, color_t);

	DrawLineEx((Vector2){x, y}, (Vector2){x_end, y_end}, thickness, branch_color);

	float new_length = length * LENGTH_REDUCTION_RATIO * (1.0f + RAND_F() * LENGTH_JITTER);
	float new_thickness = thickness * THICKNESS_REDUCTION_RATIO;

	if (new_length < LENGTH_LIMIT || new_thickness < 1.0f || depth >= maxDepth)
		return;

	// Sway increases with depth, and each level lags behind by WIND_PHASE_OFFSET
	float sway = wind * sinf(t * WIND_FREQ + depth * WIND_PHASE_OFFSET) * (1.0f + depth * WIND_DEPTH_SCALE);

	for (int i = 0; i < branchCount; i++)
	{
		float child_angle;

		if (branchCount == 1)
			child_angle = angle;
		else
			child_angle = angle + spreadAngle * (i - (branchCount - 1) / 2.0f) * (2.0f / (branchCount - 1));

		child_angle += RAND_F() * ANGLE_JITTER;
		child_angle += sway;

		DrawBranch(x_end, y_end, new_length, child_angle, new_thickness, spreadAngle, branchCount, depth + 1, maxDepth, wind, t);
	}
}