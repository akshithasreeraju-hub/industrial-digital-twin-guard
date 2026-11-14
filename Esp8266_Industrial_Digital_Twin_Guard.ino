#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// ---------- WiFi ----------
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ---------- ThingSpeak ----------
const char* apiKey = "YOUR_THINGSPEAK_WRITE_API_KEY";
const char* server = "http://api.thingspeak.com/update";

// Arduino D9 (TX) -> ESP8266 RX
// Arduino D8 (RX) <- ESP8266 TX
// SoftwareSerial baud rate: 9600

String line = "";

void setup() {
  Serial.begin(9600);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println("Industrial Digital Twin Guard");
  Serial.print("Connecting to WiFi");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi not connected.");
  }
}

void uploadToThingSpeak(
  String gas,
  String currentRaw,
  String voltageRaw,
  String temperature,
  String vibration,
  String ir,
  String water
) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected; data not uploaded.");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  String url = String(server) +
    "?api_key=" + apiKey +
    "&field1=" + gas +
    "&field2=" + currentRaw +
    "&field3=" + voltageRaw +
    "&field4=" + temperature +
    "&field5=" + vibration +
    "&field6=" + ir +
    "&field7=" + water;

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.print("ThingSpeak response: ");
    Serial.println(http.getString());
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
}

void loop() {
  if (Serial.available()) {
    line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) {
      return;
    }

    Serial.print("Received: ");
    Serial.println(line);

    // Split 7 comma-separated values
    String values[7];
    int start = 0;
    int count = 0;

    for (int i = 0; i <= line.length(); i++) {
      if (line.charAt(i) == ',' || i == line.length()) {
        if (count < 7) {
          values[count] = line.substring(start, i);
          count++;
        }
        start = i + 1;
      }
    }

    if (count == 7) {
      uploadToThingSpeak(
        values[0],
        values[1],
        values[2],
        values[3],
        values[4],
        values[5],
        values[6]
      );
    } else {
      Serial.println("Invalid sensor data format.");
    }
  }

  delay(10);
}
