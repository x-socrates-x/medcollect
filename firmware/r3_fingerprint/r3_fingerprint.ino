
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX (green), TX (white) to sensor
Adafruit_Fingerprint finger(&mySerial);

void setup() {
  Serial.begin(9600); // Communicate with R4
  while (!Serial); delay(100);

  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("FINGERPRINT_SENSOR_READY");
  } else {
    Serial.println("SENSOR_ERROR");
    while (1); // Halt if sensor isn't found
  }

  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.println("NO_TEMPLATES");
  } else {
    Serial.print("TEMPLATES_LOADED: ");
    Serial.println(finger.templateCount);
  }

  Serial.println("AUTHENTICATING..");

  // Keep retrying until fingerprint is found
  while (true) {
    int result = authenticateFingerprint();
    if (result == FINGERPRINT_OK) {
      Serial.println("ACCESS_GRANTED");
      break; // Exit loop
    } else if (result == FINGERPRINT_NOTFOUND) {
      Serial.println("ACCESS_DENIED.  Try again...");
      delay(2000);
      Serial.println("AUTHENTICATING..");
    } else {
      Serial.print("ERROR CODE: ");
      Serial.println(result);
      delay(2000);
      Serial.println("AUTHENTICATING..");
    }
    delay(3000); // Wait 3 seconds before retrying
  }
}

void loop() {
  // Nothing here - all logic is in setup()
}

int authenticateFingerprint() {
  int p = -1;

  while (p != FINGERPRINT_OK && p != FINGERPRINT_NOTFOUND) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) continue;
    else if (p == FINGERPRINT_PACKETRECIEVEERR) return FINGERPRINT_PACKETRECIEVEERR;
    else if (p != FINGERPRINT_OK) return p;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return p;

  return FINGERPRINT_OK;
}
