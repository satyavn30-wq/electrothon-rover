#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

/* ---------------- WIFI AP ---------------- */
const char* ssid = "ESP32-Robot";
const char* password = "12345678";

/* ---------------- MOTOR PINS ---------------- */
#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14

/* ---------------- MOISTURE + SERVO ---------------- */
#define MOISTURE_PIN 34   // ADC only pin
#define SERVO_PIN 13

/* --------- CALIBRATION VALUES (CHANGE IF NEEDED) ---------
   These values are SAFE defaults for most capacitive sensors
   Dry soil / air  ≈ 3800
   Wet soil        ≈ 1600
-----------------------------------------------------------*/
#define DRY_VALUE 3800
#define WET_VALUE 1600

WebServer server(80);
Servo probeServo;

/* ---------------- MOTOR CONTROL ---------------- */
void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeft() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

/* ---------------- MOISTURE HANDLER ---------------- */
void handleMoisture() {

  // Attach servo only when needed
  probeServo.attach(SERVO_PIN, 500, 2400);
  probeServo.write(90);      // Deploy probe
  delay(5000);               // Let probe settle in soil

  // ADC averaging for stability
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(MOISTURE_PIN);
    delay(50);
  }
  int raw = sum / 10;

  // Convert ADC to percentage
  int moisture = map(raw, DRY_VALUE, WET_VALUE, 0, 100);
  moisture = constrain(moisture, 0, 100);

  // Retract probe
  probeServo.write(0);
  delay(500);
  probeServo.detach();

  // Send response
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", String(moisture));

  // Debug info
  Serial.print("RAW ADC: ");
  Serial.print(raw);
  Serial.print(" | Moisture: ");
  Serial.print(moisture);
  Serial.println("%");
}

/* ---------------- SETUP ---------------- */
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();

  WiFi.softAP(ssid, password);
  Serial.print("ESP32 AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/forward", moveForward);
  server.on("/backward", moveBackward);
  server.on("/left", turnLeft);
  server.on("/right", turnRight);
  server.on("/stop", stopMotors);
  server.on("/moisture", handleMoisture);

  server.begin();
}

/* ---------------- LOOP ---------------- */
void loop() {
  server.handleClient();
}
Add ESP32 rover control code
