const int MAX_DEVICE = 256;
const int MAX_ROOM = 52;

struct DetectedDevice
{
    int id;
    int power;
    int tx;
    int ty;
};

extern int scan_device(int mDeviceId, int mScanPower, DetectedDevice mDetected[]);

#define ABS(x) (((x) > 0) ? (x) : -(x))

// Device info
struct Device {
    int id;
    int power;
};

// Global variables
static Device devices[MAX_DEVICE];
static int deviceCount;
static DetectedDevice detected[MAX_DEVICE];

// Union-Find with hash table
static const int HASH_SIZE = 671133;
static int hashId[HASH_SIZE];
static int hashParent[HASH_SIZE];

static int getHashIndex(int id) {
    int idx = id % HASH_SIZE;
    while (true) {
        if (hashId[idx] == -1) return -1;
        if (hashId[idx] == id) return idx;
        idx = (idx + 1) % HASH_SIZE;
    }
}

static int findParent(int id) {
    int idx = getHashIndex(id);
    if (idx == -1) return -1;

    if (hashParent[idx] == id) {
        return id;
    }

    return findParent(hashParent[idx]);
}

static bool createDevice(int id) {
    int idx = getHashIndex(id);
    if (idx != -1) return true;

    idx = id % HASH_SIZE;
    while (hashId[idx] != -1) {
        idx = (idx + 1) % HASH_SIZE;
    }

    hashId[idx] = id;
    hashParent[idx] = id;

    devices[deviceCount].id = id;
    devices[deviceCount].power = 0;
    deviceCount++;

    return false;
}

static void unionDevices(int id1, int id2) {
    int p1 = findParent(id1);
    int p2 = findParent(id2);

    if (p1 == p2) return;

    if (p1 > p2) {
        int idx = getHashIndex(p2);
        hashParent[idx] = p1;
    } else {
        int idx = getHashIndex(p1);
        hashParent[idx] = p2;
    }
}

static bool isSameParent(int id1, int id2) {
    return findParent(id1) == findParent(id2);
}

static Device* getNextDevice() {
    int bestIdx = 0;
    int bestScore = 1000000000;

    for (int i = deviceCount - 1; i >= 0; i--) {
        // Penalize high power devices more to reduce cost
        int score = (devices[i].power / 2) - (i == deviceCount - 1 ? 25 : 0);

        if (score < bestScore) {
            bestScore = score;
            bestIdx = i;
        }
    }

    return &devices[bestIdx];
}

static void scanFromDevice(Device* device, int scanPower) {
    int count = scan_device(device->id, scanPower, detected);

    for (int i = 0; i < count; i++) {
        createDevice(detected[i].id);

        int remainPower = ABS(scanPower - detected[i].power);
        int manhattan = ABS(detected[i].tx) + ABS(detected[i].ty);

        bool sameRoom = (manhattan == remainPower) && (detected[i].power > 1);

        if (sameRoom) {
            if (!isSameParent(detected[i].id, device->id)) {
                unionDevices(detected[i].id, device->id);
            }
        }
    }
}

void scan(int mDeviceId, int mTotalDevice)
{
    // Initialize
    deviceCount = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        hashId[i] = -1;
        hashParent[i] = -1;
    }
    for (int i = 0; i < MAX_DEVICE; i++) {
        devices[i].id = -1;
        devices[i].power = 0;
    }

    // Add first device
    createDevice(mDeviceId);

    // Phase 1: Discover all devices
    while (deviceCount < mTotalDevice) {
        Device* device = getNextDevice();
        device->power++;
        scanFromDevice(device, device->power);
    }

    // Phase 2: Merge separated groups
    // Focus on parent devices for efficiency
    for (int i = 0; i < deviceCount; i++) {
        Device* device = &devices[i];
        int parentId = findParent(device->id);

        if (parentId == device->id) {
            int targetPower = 173;
            if (device->power < targetPower) {
                scanFromDevice(device, targetPower);
            }
        } else if (device->power <=  5) {
            // Only scan child devices with extremely low power
            int targetPower = 113;
            if (device->power < targetPower) {
                scanFromDevice(device, targetPower);
            }
        }
    }
}

void result(int mDeviceIds[][MAX_DEVICE])
{
    int roomCount = 0;
    int roomParents[MAX_ROOM];
    int roomSizes[MAX_ROOM];

    for (int i = 0; i < MAX_ROOM; i++) {
        roomSizes[i] = 0;
        for (int j = 0; j < MAX_DEVICE; j++) {
            mDeviceIds[i][j] = -1;
        }
    }

    for (int i = 0; i < deviceCount; i++) {
        int deviceId = devices[i].id;
        int parentId = findParent(deviceId);

        int roomIdx = -1;
        for (int j = 0; j < roomCount; j++) {
            if (roomParents[j] == parentId) {
                roomIdx = j;
                break;
            }
        }

        if (roomIdx == -1) {
            roomIdx = roomCount;
            roomParents[roomCount] = parentId;
            roomCount++;
        }

        mDeviceIds[roomIdx][roomSizes[roomIdx]++] = deviceId;
    }

    // Sort devices within each room
    for (int i = 0; i < roomCount; i++) {
        for (int j = 0; j < roomSizes[i] - 1; j++) {
            for (int k = j + 1; k < roomSizes[i]; k++) {
                if (mDeviceIds[i][j] > mDeviceIds[i][k]) {
                    int temp = mDeviceIds[i][j];
                    mDeviceIds[i][j] = mDeviceIds[i][k];
                    mDeviceIds[i][k] = temp;
                }
            }
        }
    }

    // Sort rooms by first device ID
    for (int i = 0; i < roomCount - 1; i++) {
        for (int j = i + 1; j < roomCount; j++) {
            if (mDeviceIds[i][0] > mDeviceIds[j][0]) {
                for (int k = 0; k < MAX_DEVICE; k++) {
                    int temp = mDeviceIds[i][k];
                    mDeviceIds[i][k] = mDeviceIds[j][k];
                    mDeviceIds[j][k] = temp;
                }
            }
        }
    }
}
