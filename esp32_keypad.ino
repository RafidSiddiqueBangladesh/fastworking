#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Server URL
const char* serverUrl = "https://your-render-app.onrender.com/api/transaction"; // Update with your Render URL

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Keypad pins
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {16, 17, 18, 19}; // R1 R2 R3 R4
byte colPins[COLS] = {23, 5, 4, 32};   // C1 C2 C3 C4

String inputBuffer = "";
bool isBuying = false;
bool isSelling = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println("Connected to WiFi");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  for (byte i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  for (byte i = 0; i < COLS; i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], HIGH);
  }
}

void loop() {
  char key = getKey();
  if (key != 0) {
    handleKey(key);
  }
  delay(100);
}

char getKey() {
  for (byte c = 0; c < COLS; c++) {
    digitalWrite(colPins[c], LOW);
    for (byte r = 0; r < ROWS; r++) {
      if (digitalRead(rowPins[r]) == LOW) {
        delay(50); // debounce
        if (digitalRead(rowPins[r]) == LOW) {
          digitalWrite(colPins[c], HIGH);
          return keys[r][c];
        }
      }
    }
    digitalWrite(colPins[c], HIGH);
  }
  return 0;
}

void handleKey(char key) {
  if (key == 'A') {
    // Get sells
    getData("sells");
  } else if (key == '*') {
    // Start buying
    inputBuffer = "*";
    isBuying = true;
    isSelling = false;
    displayInput();
  } else if (key == '#') {
    // Start selling
    inputBuffer = "#";
    isSelling = true;
    isBuying = false;
    displayInput();
  } else if (key == 'D') {
    // Send data
    if (inputBuffer.length() > 1) {
      sendData(inputBuffer);
      inputBuffer = "";
      isBuying = false;
      isSelling = false;
    }
  } else if (isBuying || isSelling) {
    inputBuffer += key;
    displayInput();
  }
}

void displayInput() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(inputBuffer);
  display.display();
}

void sendData(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    String jsonData = "{\"data\":\"" + data + "\"}";
    int httpResponseCode = http.POST(jsonData);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Sent");
      display.display();
      delay(2000);
    } else {
      Serial.println("Error sending");
    }
    http.end();
  }
}

void getData(String type) {
  String url = "https://your-render-app.onrender.com/api/" + type;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      display.clearDisplay();
      display.setCursor(0,0);
      display.println(type + ":");
      display.println(payload);
      display.display();
      delay(5000);
    } else {
      Serial.println("Error getting data");
    }
    http.end();
  }
}