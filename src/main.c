#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

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

#define MAX_DEPTH 12
#define AUTO_DEPTH_CAP(bc) ((bc) <= 2 ? MAX_DEPTH : (bc) <= 3 ? 8 \
												: (bc) <= 4	  ? 6 \
															  : 5)

#define WIND_BASE RAD(1.5f)
#define WIND_GUST_STRENGTH RAD(3.0f)
#define WIND_FREQ 1.2f
#define WIND_PHASE_OFFSET 0.4f
#define WIND_DEPTH_SCALE 0.25f

#define CYCLE_DURATION 120.0f
#define NUM_SEASONS 4

#define LEAF_CLUSTER_RADIUS 18.0f
#define LEAF_CLUSTER_COUNT 6
#define LEAF_CLUSTER_SPREAD 12.0f
#define LEAF_ALPHA 180

typedef struct
{
	Color trunk;
	Color leaf;
	const char *name;
} Season;

static const Season SEASONS[NUM_SEASONS] = {
	{(Color){133, 94, 66, 255}, (Color){255, 182, 193, 255}, "Spring"},
	{(Color){101, 67, 33, 255}, (Color){34, 139, 34, 255}, "Summer"},
	{(Color){90, 60, 30, 255}, (Color){204, 85, 0, 255}, "Autumn"},
	{(Color){100, 100, 110, 255}, (Color){220, 235, 245, 255}, "Winter"},
};

// --- PROTOTYPES ------------>

float HashF(int seed, int idx);
void DrawLeafCluster(float x, float y, Color leafColor, float wind, float t, int depth, int seed, float clusterAlpha);
void DrawBranch(float x, float y, float length, float angle, float thickness,
				float spreadAngle, int branchCount, int depth, int maxDepth,
				float wind, float t, Color trunkColor, Color leafColor, float clusterAlpha, int parentSeed);

// --- ENTRY POINT ------------>

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "Fractal Tree");
	SetTargetFPS(60);

	float spreadAngle = INITIAL_SPREAD_ANGLE;
	int branchCount = INITIAL_BRANCH_COUNT;

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

		// --- SEASON ------------>

		float t = (float)GetTime();
		float cycle = fmodf(t, CYCLE_DURATION) / CYCLE_DURATION;
		float sSweep = cycle * NUM_SEASONS;
		int sIdx = (int)sSweep % NUM_SEASONS;
		int sNext = (sIdx + 1) % NUM_SEASONS;
		float sBlend = sSweep - (int)sSweep;

		Color trunkColor = ColorLerp(SEASONS[sIdx].trunk, SEASONS[sNext].trunk, sBlend);
		Color leafColor = ColorLerp(SEASONS[sIdx].leaf, SEASONS[sNext].leaf, sBlend);

		float clusterAlpha;
		if (sIdx == 2 && sNext == 3)
			clusterAlpha = 1.0f - sBlend; // autumn -> winter
		else if (sIdx == 3 && sNext == 0)
			clusterAlpha = sBlend; // winter -> spring
		else
			clusterAlpha = 1.0f;

		// --- WIND ------------>

		float gust = sinf(t * 0.7f) * sinf(t * 1.3f);
		float wind = WIND_BASE + gust * WIND_GUST_STRENGTH;

		// --- DRAW ------------>

		srand(seed);

		BeginDrawing();
		ClearBackground(BLACK);

		DrawBranch(
			WIDTH / 2, HEIGHT - 20,
			INITIAL_LENGTH, 0.0f, INITIAL_THICKNESS,
			spreadAngle, branchCount, 0,
			AUTO_DEPTH_CAP(branchCount),
			wind, t, trunkColor, leafColor, clusterAlpha, (int)seed);

		DrawText(TextFormat("Spread: %.0f deg (UP/DOWN)", spreadAngle / DEG2RAD), 10, 10, 18, GRAY);
		DrawText(TextFormat("Branches: %d (W/S)", branchCount), 10, 32, 18, GRAY);
		DrawText("SPACE: new tree   R: reset", 10, 54, 18, GRAY);
		DrawText(TextFormat("%s -> %s", SEASONS[sIdx].name, SEASONS[sNext].name), WIDTH - 160, 10, 18, GRAY);
		DrawRectangle(WIDTH - 160, 34, 150, 8, DARKGRAY);
		DrawRectangle(WIDTH - 160, 34, (int)(sBlend * 150), 8, LIGHTGRAY);

		EndDrawing();
	}

	CloseWindow();
	return 0;
}

// --- IMPLEMENTATIONS ------------>

float HashF(int seed, int idx)
{
	int h = seed * 2749 + idx * 1013;
	h = (h ^ (h >> 13)) * 1540483477;
	h = h ^ (h >> 15);
	return (float)(h & 0xFFFF) / 65535.0f * 2.0f - 1.0f;
}

void DrawLeafCluster(float x, float y, Color leafColor, float wind, float t, int depth, int seed, float clusterAlpha)
{
	float sway = wind * sinf(t * WIND_FREQ + depth * WIND_PHASE_OFFSET) * (1.0f + depth * WIND_DEPTH_SCALE);

	for (int i = 0; i < LEAF_CLUSTER_COUNT; i++)
	{
		float ox = HashF(seed, i * 2) * LEAF_CLUSTER_SPREAD + sway * 8.0f;
		float oy = HashF(seed, i * 2 + 1) * LEAF_CLUSTER_SPREAD;
		float r = LEAF_CLUSTER_RADIUS * (0.5f + (HashF(seed, i * 3) + 1.0f) * 0.35f);

		Color c = leafColor;
		c.a = (unsigned char)(LEAF_ALPHA * clusterAlpha);

		DrawCircle((int)(x + ox), (int)(y + oy), r, c);
	}
}

void DrawBranch(float x, float y, float length, float angle, float thickness,
				float spreadAngle, int branchCount, int depth, int maxDepth,
				float wind, float t, Color trunkColor, Color leafColor, float clusterAlpha, int parentSeed)
{
	float x_end = x + sinf(angle) * length;
	float y_end = y - cosf(angle) * length;

	float color_t = (float)depth / (float)MAX_DEPTH;
	if (color_t > 1.0f)
		color_t = 1.0f;
	Color branch_color = ColorLerp(trunkColor, leafColor, color_t);

	DrawLineEx((Vector2){x, y}, (Vector2){x_end, y_end}, thickness, branch_color);

	float new_length = length * LENGTH_REDUCTION_RATIO * (1.0f + RAND_F() * LENGTH_JITTER);
	float new_thickness = thickness * THICKNESS_REDUCTION_RATIO;

	if (new_length < LENGTH_LIMIT || new_thickness < 1.0f || depth >= maxDepth)
	{
		if (clusterAlpha > 0.0f)
			DrawLeafCluster(x_end, y_end, leafColor, wind, t, depth, parentSeed, clusterAlpha);
		return;
	}

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

		int childSeed = parentSeed * 31 + i;

		DrawBranch(x_end, y_end, new_length, child_angle, new_thickness,
				   spreadAngle, branchCount, depth + 1, maxDepth,
				   wind, t, trunkColor, leafColor, clusterAlpha, childSeed);
	}
}