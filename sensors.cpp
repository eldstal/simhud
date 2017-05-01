#include "sensors.hpp"

#include <iostream>
#include <ctime>
#include <cmath>

#define FAKE_SENSORS 1

using std::cerr;
using std::endl;

// 
template <typename T>
class SineSource {
  private:
    int period;
    T norm;
    T amplitude;
    T jitter;

  public:

  // Period in ms,
  // norm is vertical offset
  SineSource(int period, T norm, T amplitude, T jitter)
  : period(period),
    norm(norm),
    amplitude(amplitude),
    jitter(jitter) { }

  T next() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    long ms = t.tv_sec * 1000;
    ms += t.tv_nsec / 1e6;
    return next(ms % period);
  }

  T next(long time) {
    T sine = amplitude * sin(((double) time * 2 * M_PI) / (double) period);
    T noise = (rand() - 0.5) * jitter * 2;
    return norm + sine + jitter;
  }
};


template <typename T>
class ImpulseSource {
  private:
    int period;
    T norm;
    T height;
    T width;
    T jitter;

  public:

  ImpulseSource(int period, T norm, T height, T width, T jitter)
  : period(period),
    norm(norm),
    height(height),
    width(width),
    jitter(jitter) { }

  T next() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    long ms = t.tv_sec * 1000;
    ms += t.tv_nsec / 1e6;
    return next(ms % period);
  }

  T next(long time) {
    T spike = (time % (long) (period + width)) > width ? height : 0;
    T noise = (rand() - 0.5) * jitter * 2;
    return norm + spike + jitter;
  }

};



void query_sensors(SensorValues& out) {
  query_temp_ext(out);
  query_depth(out);
  query_power(out);
  query_acc(out);
  query_gyro(out);
  query_heading(out);
  query_radar(out);
  query_internal(out);
}


#ifdef FAKE_SENSORS
void query_temp_ext(SensorValues& out) {
  static SineSource<float> s(60000, 20, 5, 1);
  out.temp_external = s.next();
}

void query_depth(SensorValues& out) {
  static SineSource<float> s(45000, 1.5, 0.4, 0);
  out.depth = s.next();
}

void query_power(SensorValues& out) {
  static SineSource<float> u(1450, 12,  0.1, 0.3);
  static SineSource<float> i(2450, 3.0, 0.4, 0.3);
  out.power.voltage = u.next();
  out.power.current = i.next();
}

void query_acc(SensorValues& out) {
  static ImpulseSource<float> x(3000, 0,     1,  500, 0.02);
  static ImpulseSource<float> y(2000, -9.81, 10, 100, 0.02);
  static ImpulseSource<float> z(2000, 0,     1,  -100, 0.02);
  out.acc.x = x.next();
  out.acc.y = y.next();
  out.acc.z = z.next();
}

void query_gyro(SensorValues& out) {
  static ImpulseSource<float> x(3000, 0,     1,  500, 0.02);
  static ImpulseSource<float> y(2000, -9.81, 10, 100, 0.02);
  static ImpulseSource<float> z(2000, 0,     1,  -100, 0.02);
  out.gyro.x = x.next();
  out.gyro.y = y.next();
  out.gyro.z = z.next();
}

void query_heading(SensorValues& out) {
}

void query_radar(SensorValues& out) {
  static int max = 130;
  static ImpulseSource<int> d[9] = {
    ImpulseSource<int>(4000, max-30, 20,   500, 10),
    ImpulseSource<int>(5200, max-80, 30,  1010, 10),
    ImpulseSource<int>(6100, max-50, -9,  1200, 10),
    ImpulseSource<int>(3000, max-20, 10,  1000, 10),
    ImpulseSource<int>(4600, max-70, -20, 1080, 10),
    ImpulseSource<int>(6000, max-25, 20,  1300, 5),
    ImpulseSource<int>(5200, max-30, 10,  1020, 10),
    ImpulseSource<int>(3000, max-35, 15,  1000, 10),
    ImpulseSource<int>(900,  max-30, -30, 1000, 10)
  };

  out.radar.max = max;
  out.radar.fov = 70;
  for (int i=0; i<SENSOR_RADAR_STEPS; ++i) {
    out.radar.dist[i] = d[i % 9].next();
  }
}

void query_internal(SensorValues& out) {
  static SineSource<float>   t(60000, 21, 4, 1);
  static SineSource<uint8_t> h(80000, 70, 10, 2);
  out.internal.temp = t.next();
  out.internal.humidity = h.next();
}

#else

void query_temp_ext(SensorValues& out) {
}

void query_depth(SensorValues& out) {
}

void query_power(SensorValues& out) {
}

void query_acc(SensorValues& out) {
}

void query_gyro(SensorValues& out) {
}

void query_heading(SensorValues& out) {
}

void query_radar(SensorValues& out) {

}

void query_internal(SensorValues& out) {
}

#endif
