#include "arduino_secrets.h"

#include <WiFiS3.h>
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

char ssid[] = SECRET_SSID;        // set in arduino_secrets.h
char pass[] = SECRET_PASS;        // set in arduino_secrets.h

WiFiServer server(80);
String lastStatus = "Waiting...";
const int heartPin = A0;

void setup() {
  // Reset the R3
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW); 
  delay(3000); 
  digitalWrite(7, HIGH);  

  Serial.begin(9600);      // For debugging
  Serial1.begin(9600);     // Communication with R3

  // LCD setup
  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255);
  lcd.print("Connecting WiFi");

  // Connect to Wi-Fi
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  delay(2000);
  lcd.clear();
  lcd.print("Waiting for R3");

  server.begin();
}

void loop() {
  // 1. Listen for messages from R3
  if (Serial1.available()) {
    String message = Serial1.readStringUntil('\n');
    message.trim();

    Serial.println("From R3: " + message);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message.substring(0, 16));
    if (message.length() > 16) {
      lcd.setCursor(0, 1);
      lcd.print(message.substring(16, 32));
    }

    if (message == "ACCESS_GRANTED") {
      lastStatus = "Connection Successful";
      delay(3000);
      lcd.clear();
      lcd.print("*Refer to App*");
    }
  }
  // 2. Handle HTTP requests from mobile app
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.read(); // consume newline

    if (request.indexOf("GET /updateStatus") >= 0) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println();
      client.println(lastStatus);
    }

    else if (request.indexOf("GET /sayHi") >= 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("App Says Hi");
      lastStatus = "App Greeted";

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println();
      client.println("LCD Updated");
    }

    else if (request.indexOf("GET /collectData") >= 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Reading Pulse...");

      int heartRate = readHeartRate();
      lastStatus = "Pulse: " + String(heartRate);

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println();
      client.println(String(heartRate));  // Only return raw heart rate value
      delay(3000);
      lcd.clear();
      lcd.print("*Refer to App*");
    }
    else if (request.indexOf("GET /closeApp") >= 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Connection Lost..");
      delay(3000);
      lcd.clear();
      lcd.print("*Restart Device*");
    }
    else {
      client.println("HTTP/1.1 404 Not Found");
      client.println();
    }

    delay(1);
    client.stop();
  }
}

int readHeartRate() {
  long total = 0;
  const int samples = 100;
  for (int i = 0; i < samples; i++) {
    total += analogRead(heartPin);
    delay(5);
  }
  int average = total / samples;
  return average;  // Raw value (0-1023). You can calibrate this in the app.
}
