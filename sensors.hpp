#pragma once
#include <inttypes.h>

typedef struct {
  float   temp_external;     // Degrees C
  float   depth;             // m

  struct {
    float   current;         // A
    float   voltage;         // V
  } power;

  struct {
    float x, y, z;
  } acc;                  // Acceleration on each axis

  struct {
    float x, y, z;
  } gyro;                 // Rotational velocity about each axis

  struct {
    float x, y, z;        // Compass reading
  } heading;

  int range[32];          // Distance forward at each angle

  struct {
    float   temp;         // Degrees C
    uint8_t humidity;     // Percent
    bool    leak;         // True if water detected inside hull
  } internal;

  uint32_t sensor_period; // Time taken to query all sensors

  uint32_t flags;         // Filled by SensorFlags (see below)

} SensorValues;

enum SensorFlags {
  ErrorAcc  = 0x1 << 0,     // Accelerometer failure
  ErrorTemp = 0x1 << 1      // Temperature sensor failure
};

void query_sensors(SensorValues& out);

void query_temp_ext(SensorValues& out);
void query_depth(SensorValues& out);
void query_power(SensorValues& out);
void query_acc(SensorValues& out);
void query_gyro(SensorValues& out);
void query_heading(SensorValues& out);
void query_radar(SensorValues& out);
void query_internal(SensorValues& out);
