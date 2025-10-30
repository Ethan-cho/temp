/// user.cpp ///
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

void scan(int mDeviceId, int mTotalDevice)
{
    return;
}

void result(int mDeviceIds[][MAX_DEVICE])
{
    return;
}
