#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS33.h>
#include <Adafruit_LIS3MDL.h>

Adafruit_LSM6DS33 imu;
Adafruit_LIS3MDL    mag;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Wire.begin();

  // ---- accel/gyro at 0x6B ----
  if (!imu.begin_I2C(0x6B)) {
    Serial.println("ERROR: LSM6DS33 not found @0x6B");
    while (1);
  }
  Serial.println("LSM6DS33 found @0x6B");

  // ---- magnetometer @0x1E (default) ----
  if (!mag.begin_I2C(0x1E)) {
    Serial.println("ERROR: LIS3MDL not found");
    while (1);
  }
  Serial.println("LIS3MDL found");
}

void loop() {
  sensors_event_t a, g, t, m;
  imu.getEvent(&a, &g, &t);
  mag.getEvent(&m);

  // — Accelerometer (m/s²) —
  Serial.print("A:");
  Serial.print(a.acceleration.x, 2); Serial.print(',');
  Serial.print(a.acceleration.y, 2); Serial.print(',');
  Serial.print(a.acceleration.z, 2);

  // — Gyro (rad/s) —
  Serial.print("  G:");
  Serial.print(g.gyro.x, 2); Serial.print(',');
  Serial.print(g.gyro.y, 2); Serial.print(',');
  Serial.print(g.gyro.z, 2);

  // — Magnetometer (µT) —
  Serial.print("  M:");
  Serial.print(m.magnetic.x, 1); Serial.print(',');
  Serial.print(m.magnetic.y, 1); Serial.print(',');
  Serial.print(m.magnetic.z, 1);


//  // 1) roll & pitch
//  float roll  = atan2(a.acceleration.y,  a.acceleration.z);
//  float pitch = atan2(-a.acceleration.x, 
//                      sqrt(a.acceleration.y*a.acceleration.y 
//                          + a.acceleration.z*a.acceleration.z));
//
//  // 2) tilt-compensated heading
//  float Xh = m.magnetic.x *  cos(pitch)
//           + m.magnetic.z *  sin(pitch);
//  float Yh = m.magnetic.x *  sin(roll)*sin(pitch)
//           + m.magnetic.y *  cos(roll)
//           - m.magnetic.z *  sin(roll)*cos(pitch);
//  float heading = atan2(-Yh, Xh);
//  float yaw = heading * 180.0/PI;
//  if (yaw < 0) yaw += 360;
//
//  // 3) print in degrees
//  Serial.print("Roll: ");  Serial.print(roll  * 180/PI, 1);
//  Serial.print("  Pitch: "); Serial.print(pitch * 180/PI, 1);
//  Serial.print("  Yaw: ");   Serial.println(yaw, 1);

  Serial.println();
  delay(200);
}
