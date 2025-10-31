/*
* sample_input.txt (main 코드 수정 TC = 30) : 19483229383 점
* total_input.txt (main 코드 수정 TC = 105) : 68560964113 점
* 
1. device가 나중 추가되는게 새로 발견된 디바이스
2. hash[A][B] , hashI[A] 를 활용 충돌 해결
3. 일규님 아이디어 추가 1-> 마지막에 device별로 적절한 범위에 전체 스캔을 해서 분리된 그룹을 합침
4. 일규님 아이디어 추가 2-> 아이디가 높을 수록 집합의 parent가 되도록함
5. 파워가 너무 낮은 경우, 마지막에 추가된 녀석을 기준으로 뒤부터 처리
*/
#define ABS(x) (((x)<0)?(-(x)):(x))
#define MIN(x,y) (((x)<(y))?((x)):(y))
#define MAX(x,y) (((x)<(y))?((y)):(x))
struct Detected {
    int id;
    int power;
    int tx;
    int ty;
};
typedef struct Device {
    int id;
    int power;
} Device;
extern int scan_device(int mDeviceId, int mPower, struct Detected mDetected[]);

Detected mDetected[256];
Device devices[256];
int dI = 0;
int parent[671133][2]; // 0 : real id, 1 : parent real id

inline int getParentIndex(int id) {
    int pI = id % 671133;
    while (true) {
        if (parent[pI][0] == -1) return -1;
        if (parent[pI][0] == id) return pI;
        pI = (pI + 1) % 671133;
    }
}

int getParentId(int id) {
    int pI = getParentIndex(id);
    return parent[pI][1] == id ? id : getParentId(parent[pI][1]);
}

bool createPa(int id) {
    if (getParentIndex(id) == -1) {
        int pI = id % 671133;
        while (parent[pI][0] != -1) pI = (pI + 1) % 671133;
        parent[pI][0] = id;
        parent[pI][1] = id;

        devices[dI].id = id;
        devices[dI].power = 0;
        dI++;
        return false;
    }
    else return true;
}

void unify(int id, int id2) {
    int p1 = getParentId(id);
    int p2 = getParentId(id2);
    if (p2 > p1) {
        parent[getParentIndex(p1)][1] = p2;
    } else {
        parent[getParentIndex(p2)][1] = p1;
    }
}

bool isSameParent(int id, int id2) {
    return getParentId(id) == getParentId(id2);
}

Device& getNext() {
    int minI = 0 , min = 1e9;
    for (int i = dI - 1; i >= 0; i--) {
        // 마지막에 추가된것 , 그리고 파워가 10이하로 세제곱 해도 별로 영향 없는것들
        int score = (devices[i].power / 5) - (i == dI - 1 ? 15 : 0); -(devices[i].power <= 10 ? 5 : 0);
        if (min > score) {
            min = score;
            minI = i;
        }
    }
    return devices[minI];
}


void result(int mDeviceIds[][256]) {
    int mDeviceIdsSize[52] = { 0, };
    int indexToParent[52];
    int iI = 0;
    for (int i = 0; i < 52; i++) {
        mDeviceIdsSize[i] = 0;
        for (int j = 0; j < 256; j++)
            mDeviceIds[i][j] = -1;
    }

    for (int i = 0; i < dI; i++) {
        int id = devices[i].id;
        int parentId = getParentId(id);
        int j = 0;
        for (j; j < iI; j++) {
            if (indexToParent[j] == parentId) break;
        }
        if (j == iI)
            indexToParent[iI++] = parentId;

        mDeviceIds[j][mDeviceIdsSize[j]++] = id;
    }

    for (int i = 0; i < iI; i++) {
        for (int j = 0; j < mDeviceIdsSize[i] - 1; j++) {
            for (int k = j + 1; k < mDeviceIdsSize[i]; k++) {
                if (mDeviceIds[i][j] > mDeviceIds[i][k]) {
                    int temp = mDeviceIds[i][j];
                    mDeviceIds[i][j] = mDeviceIds[i][k];
                    mDeviceIds[i][k] = temp;
                }
            }
        }
    }

    for (int i = 0; i < iI - 1; i++) {
        for (int j = i; j < iI; j++) {
            if (mDeviceIds[i][0] > mDeviceIds[j][0]) {
                for (int t = 0; t < 256; t++) {
                    int temp = mDeviceIds[i][t];
                    mDeviceIds[i][t] = mDeviceIds[j][t];
                    mDeviceIds[j][t] = temp;
                }
            }
        }
    }
}

void scanByDevice(Device& device, int power) {
    int count = scan_device(device.id, power, mDetected);
    for (int i = 0; i < count; i++) {
        createPa(mDetected[i].id);
        int remainPower = ABS(power - mDetected[i].power);
        bool isSameRoom = ABS(mDetected[i].tx) + ABS(mDetected[i].ty) == remainPower && mDetected[i].power > 1;
        if (isSameRoom) {
            if (!isSameParent(mDetected[i].id, device.id)) {
                unify(mDetected[i].id, device.id);
            }
        }
    }
}

void scan(int mDeviceId, int mDeviceNum) {
    dI = 0;
    for (int i = 0; i < 671133; i++) parent[i][0] = -1, parent[i][1] = -1;
    for (int i = 0; i < 256; i++) devices[i].id = -1, devices[i].power = 0;
    createPa(mDeviceId);
    while (dI != mDeviceNum) {
        Device& device = getNext();
        int power = ++device.power;
        scanByDevice(device, power);
    }

    for (int i = 0; i < dI; i++) {
        Device& device = devices[i];
        if (getParentId(devices[i].id) == devices[i].id) {
            int p = 196;
            if (devices[i].power < p)
                scanByDevice(device, p);
        }
        else {
            int p = 137;
            if (devices[i].power < p)
                scanByDevice(device, p);
        }
    }
    return;
}
