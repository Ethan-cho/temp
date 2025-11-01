#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>

#define ABS(x) (((x)>0)?(x):(-(x)))


static unsigned long long seed = 5;

static int pseudo_rand(void)
{
	seed = seed * 25214903917ULL + 11ULL;
	return (seed >> 16) & 0x3fffffff;
}

struct Coordinate {
	int id;
	int y;
	int x;
};

struct Detected {
	int id;
	int power;
	int tx;
	int ty;
};

/* These constants will not be changed */
static const int MAX_ROOM = 52;
static const int MAX_DEVICE = 256;
static const int MAP_SIZE = 100;
static char gMap[MAP_SIZE][MAP_SIZE];
static int mDeviceIds[MAX_ROOM][MAX_DEVICE];
static int Rooms[MAX_ROOM];
static int RoomDevices[MAX_ROOM];
static int gRoomNum;
static struct Coordinate gDevices[MAX_ROOM][MAX_DEVICE];
static struct Coordinate gDeviceId[MAX_DEVICE];
static const int TC = 30;
static long long gTotalCost;
static long long gCost;
static int gDeviceNum;
static long long PENALTY = 1'000'000'000'000;

static long long calculate_power(int power, int sy, int sx, int ey, int ex, int& ty, int& tx) {
	int isWall;

	int py = ABS(sy - ey);
	int px = ABS(sx - ex);

	int dx = (sx <= ex) ? (1) : (-1);
	int dy = (sy <= ey) ? (1) : (-1);

	isWall = false;
	for (register int y = sy; y != ey; y += dy) {
		if (!isWall && gMap[y][sx] == '#') {
			power /= 2;
			py /= 2;
			isWall = true;
		}
		else {
			power--;
			py--;
			isWall = false;
		}
	}
	if (sx != ex) {
		if (!isWall && gMap[ey][sx] == '#' && gMap[ey][sx + dx] != '#') {
			power /= 2;
			px /= 2;
		}

		isWall = false;
		for (register int x = sx; x != ex; x += dx) {
			if (!isWall && gMap[ey][x] == '#') {
				power /= 2;
				px /= 2;
				isWall = true;
			}
			else {
				power--;
				px--;
				isWall = false;
			}

		}
	}
	ty = (ey - sy) + py * dy;
	tx = (ex - sx) + px * dx;
	return (power > 0) ? power : 0;
}

int scan_device(int mDeviceId, int mPower, struct Detected mDetected[])
{
	int sy, sx, ty1, tx1, ty2, tx2, ty, tx;
	int mDetectedNum = 0;
	int power;
	int ind = -1;

	for (register int i = 0; i < gDeviceNum; ++i) {
		if (gDeviceId[i].id == mDeviceId) {
			ind = i;
			sy = gDeviceId[i].y;
			sx = gDeviceId[i].x;
			break;
		}
	}

	for (register int i = 0; i < gDeviceNum; ++i) {
		if (ind == i) continue;

		int power1 = calculate_power(mPower, sy, sx, gDeviceId[i].y, gDeviceId[i].x, ty1, tx1);
		int power2 = calculate_power(mPower, gDeviceId[i].y, gDeviceId[i].x, sy, sx, ty2, tx2);
		if (power1 >= power2) {
			power = power1;
			ty = ty1;
			tx = tx1;
		}
		else {
			power = power2;
			ty = -ty2;
			tx = -tx2;
		}

		if (power > 0) {
			mDetected[mDetectedNum].id = gDeviceId[i].id;
			mDetected[mDetectedNum].power = power;
			mDetected[mDetectedNum].ty = ty;
			mDetected[mDetectedNum].tx = tx;
			mDetectedNum++;
		}
	}
	
	gCost += (long long)mPower * mPower * mPower;
	return mDetectedNum;
}

void load_map(void)
{
	for (register int i = 0; i < MAX_ROOM; ++i) Rooms[i] = -1;
	for (register int i = 0; i < MAX_ROOM; ++i) RoomDevices[i] = 0;

	int id = 0;
	gDeviceNum = 0;
	gRoomNum = 0;
	for (register int i = 0; i < MAP_SIZE; ++i) {
		fgets(gMap[i], MAP_SIZE + 2, stdin);
		for (register int j = 0; j < MAP_SIZE; ++j) {

			int roomIndex = ('A' <= gMap[i][j] && gMap[i][j] <= 'Z') ? (gMap[i][j] - 'A') : (('a' <= gMap[i][j] && gMap[i][j] <= 'z') ? (gMap[i][j] - 'a' + ('Z' - 'A' + 1)) : (-1));

			if (roomIndex == -1) continue;

			if (Rooms[roomIndex] == -1) {
				Rooms[roomIndex] = gRoomNum++;
			}

			id += pseudo_rand() % (1024 * 1024) + 1; // 1 ~ 2^20

			gDeviceId[gDeviceNum].id = id;
			gDeviceId[gDeviceNum].y = i;
			gDeviceId[gDeviceNum].x = j;

			gDevices[Rooms[roomIndex]][RoomDevices[Rooms[roomIndex]]++] = gDeviceId[gDeviceNum];

			gDeviceNum++;
		}
	}
	// shuffling
	for (register int i = gDeviceNum - 1; i >= 0; --i) {
		int rand = pseudo_rand() % (i + 1);
		struct Coordinate temp = gDeviceId[i];
		gDeviceId[i] = gDeviceId[rand];
		gDeviceId[rand] = temp;
	}
}

long long verify(int mDeviceIds[][MAX_DEVICE]) {
	for (register int i = 0; i < gRoomNum; ++i) {
		for (register int j = 0; j < RoomDevices[i]; ++j) {
			if (gDevices[i][j].id != mDeviceIds[i][j])
				return PENALTY;
		}
	}
	return 0;
}
extern void scan(int mDeviceId, int mDeviceNum);
extern void result(int mDeviceIds[][MAX_DEVICE]);

int main(void)
{
	//setbuf(stdout, NULL);
	freopen("sample_input.txt", "r", stdin);

	gTotalCost = 0;
	for (int tc = 0; tc < TC; ++tc) {
		gCost = 0;
		load_map();
		scan(gDeviceId[0].id, gDeviceNum);

		for (register int i = 0; i < MAX_ROOM; ++i) {
			for (register int j = 0; j < MAX_DEVICE; ++j) {
				mDeviceIds[i][j] = -1;
			}
		}
		result(mDeviceIds);

		gCost += verify(mDeviceIds);
		if (gCost >= PENALTY) {
			printf("SCORE: %lld\n", PENALTY);
			return 0;
		}
		gTotalCost += gCost;
	}
	printf("SCORE: %lld\n", gTotalCost);

	return 0;
}
