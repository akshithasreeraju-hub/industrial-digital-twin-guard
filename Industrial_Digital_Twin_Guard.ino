#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET
);

// Arduino D8 = RX, D9 = TX
SoftwareSerial espSerial(8, 9);

// ---------- Sensor Pins ----------
const int MQ2_PIN = A0;
const int ACS712_PIN = A1;
const int VOLTAGE_PIN = A2;
const int LM35_PIN = A3;

const int VIBRATION_PIN = 2;
const int IR_PIN = 3;
const int WATER_PIN = 4;

// ---------- Settings ----------
const float ADC_REF = 5.0;
const int ADC_MAX = 1023;

// LM35: approximately 10mV per degree Celsius
float readTemperatureC() {
  int raw = analogRead(LM35_PIN);
  float voltage = raw * ADC_REF / ADC_MAX;
  return voltage * 100.0;
}

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(VIBRATION_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(WATER_PIN, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found. Check wiring/address.");
    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("INDUSTRIAL");
  display.println("DIGITAL TWIN GUARD");
  display.println("Starting sensors...");
  display.display();

  delay(1500);
}

void loop() {
  // Read analog sensors
  int gasRaw = analogRead(MQ2_PIN);
  int currentRaw = analogRead(ACS712_PIN);
  int voltageRaw = analogRead(VOLTAGE_PIN);
  float temperatureC = readTemperatureC();

  // Read digital sensors
  int vibration = digitalRead(VIBRATION_PIN);
  int irDetected = digitalRead(IR_PIN);
  int waterDetected = digitalRead(WATER_PIN);

  // Send comma-separated values to ESP8266
  // Format:
  // gas,currentRaw,voltageRaw,temp,vibration,IR,water
  espSerial.print(gasRaw);
  espSerial.print(",");
  espSerial.print(currentRaw);
  espSerial.print(",");
  espSerial.print(voltageRaw);
  espSerial.print(",");
  espSerial.print(temperatureC, 1);
  espSerial.print(",");
  espSerial.print(vibration);
  espSerial.print(",");
  espSerial.print(irDetected);
  espSerial.print(",");
  espSerial.println(waterDetected);

  // Debug values on Arduino Serial Monitor
  Serial.print("Gas ADC: ");
  Serial.print(gasRaw);
  Serial.print(" | Current ADC: ");
  Serial.print(currentRaw);
  Serial.print(" | Voltage ADC: ");
  Serial.print(voltageRaw);
  Serial.print(" | Temperature C: ");
  Serial.println(temperatureC, 1);

  // OLED display
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("DIGITAL TWIN GUARD");

  display.print("Gas ADC: ");
  display.println(gasRaw);

  display.print("Temp C: ");
  display.println(temperatureC, 1);

  display.print("Vibration: ");
  display.println(vibration == HIGH ? "Detected" : "Normal");

  display.print("IR: ");
  display.println(irDetected == LOW ? "Detected" : "Clear");

  display.print("Water DO: ");
  display.println(waterDetected);

  display.display();

  delay(2000);
}
