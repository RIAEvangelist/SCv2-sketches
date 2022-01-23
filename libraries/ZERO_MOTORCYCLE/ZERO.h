#ifndef ZERO_H
#define ZERO_H

#include <Arduino.h>
#include "Network.h"
#include "unions.h"


class Zero {
  public:
    Zero();
    ~Zero();

    static byte messageLength;

    byte hasVoltage(short &canId);
    byte hasMonolithVoltage(short &canId);
    byte hasPowerTankVoltage(short &canId);

    byte hasAmps(short &canId);
    byte hasMonolithAmps(short &canId);
    byte hasPowerTankAmps(short &canId);

    byte hasMaxCRate(short &canId);
    byte hasMonolithMaxCRate(short &canId);
    byte hasPowerTankMaxCRate(short &canId);

    byte hasPackTime(short &canId);
    byte hasMonolithPackTime(short &canId);
    byte hasPowerTankPackTime(short &canId);

    byte hasPackConfig(short &canId);
    byte hasMonolithPackConfig(short &canId);
    byte hasPowerTankPackConfig(short &canId);

    byte hasPackActiveData(short &canId);
    byte hasMonolithPackActiveData(short &canId);
    byte hasPowerTankPackActiveData(short &canId);

    byte hasThrottle(short &canId);

    long voltage(byte &len,byte buf[]);
    short sagAdjust(byte &len,byte buf[]);

    short amps(byte &len,byte buf[]);

    short throttle(byte &len,byte buf[]);

    short maxCRate(byte &len,byte buf[]);

    short AH(byte &len,byte buf[]);

    short highestTemp(byte &len,byte buf[]);
    short lowestTemp(byte &len,byte buf[]);

    void logInit();
    void logRaw(byte &len,byte buf[],short &canId);

};

#endif
