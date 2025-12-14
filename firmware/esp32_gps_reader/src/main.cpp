#include <Arduino.h>
#include <HardwareSerial.h>

// MAVLink header from the library (common dialect)
#include "common/mavlink.h"

// UART2 of the ESP32 for connection to the Flight Controller (TELEM2)
HardwareSerial FCSerial(2);  // UART2

// Structure to store the current GPS state
struct GpsState {
  bool valid = false;
  float lat = 0.0f;       // Latitude in degrees
  float lon = 0.0f;       // Longitude in degrees
  float alt_m = 0.0f;     // Altitude in meters (AMSL)
};

GpsState gps;

// MAVLink parser objects
mavlink_message_t msg;
mavlink_status_t status;

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=== ESP32 MAVLink GPS Reader ===");
  Serial.println("Waiting for MAVLink data from the Flight Controller...");

  // Initialize UART2:
  // RX = GPIO16, TX = GPIO17, Baud rate 57600 (common TELEM2 setting)
  FCSerial.begin(57600, SERIAL_8N1, 16, 17);
  Serial.println("UART2 started (57600 Baud, RX=16, TX=17)");
}

void loop() {
  // Read incoming MAVLink data from the Flight Controller
  while (FCSerial.available() > 0) {
    uint8_t c = FCSerial.read();

    // Feed byte into MAVLink parser
    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {

      // Debug: print every received message id
      Serial.print("MAVLink msg id: ");
      Serial.println(msg.msgid);

      // ------------------------------------------------------------------
      // 1) GLOBAL_POSITION_INT (ID 33) – fused GPS/position estimate
      // ------------------------------------------------------------------
      if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
        mavlink_global_position_int_t pos;
        mavlink_msg_global_position_int_decode(&msg, &pos);

        // Convert raw MAVLink data:
        // lat & lon: 1E7 scaling -> degrees
        // alt: millimeters -> meters
        gps.lat   = pos.lat / 1e7f;
        gps.lon   = pos.lon / 1e7f;
        gps.alt_m = pos.alt / 1000.0f;
        gps.valid = true;
      }

      // ------------------------------------------------------------------
      // 2) GPS_RAW_INT (ID 24) – raw GPS fix (alternative source)
      // ------------------------------------------------------------------
      if (msg.msgid == MAVLINK_MSG_ID_GPS_RAW_INT) {
        mavlink_gps_raw_int_t raw;
        mavlink_msg_gps_raw_int_decode(&msg, &raw);

        // Only use if we have at least a 3D fix
        if (raw.fix_type >= 3) {  // 3 = 3D fix, 4/5 = better
          gps.lat   = raw.lat / 1e7f;
          gps.lon   = raw.lon / 1e7f;
          gps.alt_m = raw.alt / 1000.0f;
          gps.valid = true;
        }
      }
    }
  }

  // Print GPS data every 1 second
  static uint32_t lastPrint = 0;
  uint32_t now = millis();

  if (now - lastPrint > 1000) {
    lastPrint = now;

    if (gps.valid) {
      Serial.print("GPS: Lat=");
      Serial.print(gps.lat, 7);
      Serial.print("  Lon=");
      Serial.print(gps.lon, 7);
      Serial.print("  Alt=");
      Serial.print(gps.alt_m, 2);
      Serial.println(" m");
    } else {
      Serial.println("GPS: No GLOBAL_POSITION_INT or GPS_RAW_INT with 3D fix received yet...");
    }
  }
}

