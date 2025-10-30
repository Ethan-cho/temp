#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>

static unsigned long long seed = 5;
static int pseudo_rand(void)
{
    seed = seed * 25214903917ULL + 11ULL;
    return (seed >> 16) & 0x3fffffff;
}

/* These constant variables will NOT be changed */
static const int MAP_SIZE = 100;
static const int MAX_DEVICE = 256;
static const int MAX_ROOM = 52;
static const long long PENALTY = 10'000'000'000'000LL;
static const int MAX_TC = 30;

struct Coordinate
{
    int id;
    int y, x;
};

struct DetectedDevice
{
    int id;
    int power;
    int tx;
    int ty;
};

static char gMap[MAP_SIZE][MAP_SIZE + 2];

static int gNumRoom;
static int gNumDevice[MAX_ROOM];
static Coordinate gDevice[MAX_ROOM][MAX_DEVICE];

static int gTotalNumDevice;
static Coordinate gMergedDevice[MAX_DEVICE];

static long long gTotalCost;

extern void scan(int mDeviceId, int mTotalDevice);
extern void result(int mDeviceIds[][MAX_DEVICE]);

#define ABS(x) (((x) > 0) ? (x) : -(x))

static int calcuate_power(int power, int sx, int sy, int ex, int ey, int& tx, int& ty)
{
    int x, y, dx, dy, px, py;

    px = ABS(ex - sx);
    py = ABS(ey - sy);

    dx = (sx <= ex) ? 1 : -1;
    dy = (sy <= ey) ? 1 : -1;

    bool isWall = false;
    for (y = sy; y != ey; y += dy)
        if (gMap[y][sx] == '#' && !isWall)
        {
            power /= 2;
            py /= 2;
            isWall = true;
        }
        else
        {
            --power;
            --py;
            isWall = gMap[y][sx] == '#';
        }

    if (sx != ex)
    {
        if (!isWall && gMap[y][sx] == '#' && gMap[y][sx + dx] != '#')
        {
            power /= 2;
            px /= 2;
        }
        else
        {
            --power;
            --px;
        }

        isWall = false;
        for (x = sx + dx; x != ex; x += dx)
            if (gMap[y][x] == '#' && !isWall)
            {
                power /= 2;
                px /= 2;
                isWall = true;
            }
            else
            {
                --power;
                --px;
                isWall = gMap[y][x] == '#';
            }
    }

    tx = (ex - sx) + px * dx;
    ty = (ey - sy) + py * dy;

    return power > 0 ? power : 0;
}

int scan_device(int deviceId, int scanPower, DetectedDevice detected[])
{
    int i, j, sx, sy, tx, ty, tx1, ty1, tx2, ty2;
    int numDevice;
    int power, pow1, pow2;

    if (scanPower <= 0 || scanPower > 1000 || gTotalCost >= PENALTY)
    {
        gTotalCost = PENALTY;
        return -1;
    }

    for (i = 0; i < gTotalNumDevice; ++i)
        if (gMergedDevice[i].id == deviceId)
            break;

    if (i == gTotalNumDevice)
    {
        gTotalCost = PENALTY;
        return -1;
    }


    sx = gMergedDevice[i].x;
    sy = gMergedDevice[i].y;

    numDevice = 0;
    for (j = 0; j < gTotalNumDevice; ++j)
    {
        if (i == j)
            continue;

        pow1 = calcuate_power(scanPower, sx, sy, gMergedDevice[j].x, gMergedDevice[j].y, tx1, ty1);
        pow2 = calcuate_power(scanPower, gMergedDevice[j].x, gMergedDevice[j].y, sx, sy, tx2, ty2);

        if (pow1 >= pow2)
        {
            power = pow1;
            tx = tx1;
            ty = ty1;
        }
        else
        {
            power = pow2;
            tx = -tx2;
            ty = -ty2;
        }

        if (power > 0)
        {
            detected[numDevice].id = gMergedDevice[j].id;
            detected[numDevice].power = power;
            detected[numDevice].tx = tx;
            detected[numDevice].ty = ty;
            ++numDevice;
        }
    }

    gTotalCost += (long long)scanPower * scanPower * scanPower;

    return numDevice;
}

static void load_map()
{
    int room[MAX_ROOM];
    int roomIndex, roomId, deviceIndex, deviceId;

    for (int i = 0; i < MAX_ROOM; ++i)
    {
        gNumDevice[i] = 0;
        room[i] = -1;
    }

    gNumRoom = 0;
    deviceId = 1;
    for (int i = 0; i < MAP_SIZE; ++i)
    {
        fgets(gMap[i], MAP_SIZE + 2, stdin);
        for (int j = 0; j < MAP_SIZE; ++j)
            if ('A' <= gMap[i][j] && gMap[i][j] <= 'Z' || 'a' <= gMap[i][j] && gMap[i][j] <= 'z')
            {
                roomId = 'A' <= gMap[i][j] && gMap[i][j] <= 'Z' ? gMap[i][j] - 'A' : gMap[i][j] - 'a' + ('Z' - 'A' + 1);

                if (room[roomId] == -1)
                    room[roomId] = gNumRoom++;

                roomIndex = room[roomId];
                deviceIndex = gNumDevice[roomIndex]++;

                deviceId += (pseudo_rand() % (1024 * 1024) + 1);

                gDevice[roomIndex][deviceIndex].id = deviceId;
                gDevice[roomIndex][deviceIndex].y = i;
                gDevice[roomIndex][deviceIndex].x = j;
            }
    }

    gTotalNumDevice = 0;
    for (int i = 0; i < gNumRoom; ++i)
        for (int j = 0; j < gNumDevice[i]; ++j)
            gMergedDevice[gTotalNumDevice++] = gDevice[i][j];

    for (int i = gTotalNumDevice - 1; i > 0; --i)
    {
        int idx = pseudo_rand() % (i + 1);
        Coordinate device_t = gMergedDevice[i];
        gMergedDevice[i] = gMergedDevice[idx];
        gMergedDevice[idx] = device_t;
    }
}

static long long verify(int deviceIds[][MAX_DEVICE])
{
    for (int i = 0; i < gNumRoom; ++i)
        for (int j = 0; j < gNumDevice[i]; ++j)
            if (gDevice[i][j].id != deviceIds[i][j])
                return PENALTY;

    return 0;
}

int main()
{
    int deviceGroup[MAX_ROOM][MAX_DEVICE];

    setbuf(stdout, NULL);
    freopen("input.txt", "r", stdin);

    gTotalCost = 0;
    for (int tc = 0; tc < MAX_TC; ++tc)
    {
        load_map();
        scan(gMergedDevice[0].id, gTotalNumDevice);

        for (int i = 0; i < MAX_ROOM; ++i)
            for (int j = 0; j < MAX_DEVICE; ++j)
                deviceGroup[i][j] = 0;

        result(deviceGroup);
        gTotalCost += verify(deviceGroup);

        if (gTotalCost >= PENALTY)
        {
            printf("SCORE: %lld\n", PENALTY);
            return 0;
        }
    }

    printf("SCORE: %lld\n", gTotalCost);

    ///////////////////////////////////////////////////////////
    if (gTotalCost > 21'000'000'000)
        puts("FAIL");
    else
        puts("PASS");
    ///////////////////////////////////////////////////////////

    return 0;
}