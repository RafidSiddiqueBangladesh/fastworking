#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Keypad.h>

// ================= WIFI =================
const char* ssid = "Abcde";
const char* password = "12345678";

// ================= SERVER =================
const char* serverUrl = "https://fastworking.onrender.com/api/transaction";

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================= KEYPAD =================
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13, 14, 25, 26};
byte colPins[COLS] = {27, 33, 32, 19};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ================= STATE =================
String inputBuffer = "";
bool isBuying = false;
bool isSelling = false;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32 START ===");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi CONNECTED");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("READY");
  display.display();
}

// ================= LOOP =================
void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    handleKey(key);
  }
}

// ================= LOGIC =================
void handleKey(char key) {

  if (key == 'A') {
    getData("sells-summary");
  }
  else if (key == 'B') {
    getData("buys-summary");
  }
  else if (key == 'C') {
    getData("revenue-summary");
  }
  else if (key == '*') {
    inputBuffer = "*";
    isBuying = true;
    isSelling = false;
    displayInput();
  }
  else if (key == '#') {
    inputBuffer = "#";
    isSelling = true;
    isBuying = false;
    displayInput();
  }
  else if (key == 'D') {
    if (inputBuffer.length() > 1) {
      sendData(inputBuffer);
      inputBuffer = "";
      isBuying = false;
      isSelling = false;
    }
  }
  else if (isBuying || isSelling) {
    inputBuffer += key;
    displayInput();
  }
}

// ================= DISPLAY =================
void displayInput() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println(inputBuffer);
  display.display();
}

// ================= HTTP POST WITH DEBUG =================
void sendData(String data) {

  Serial.println("\n--- SEND DATA ---");
  Serial.print("Payload: ");
  Serial.println(data);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ FAIL: WiFi DISCONNECTED");
    showStatus("NO WIFI");
    return;
  }

  HTTPClient http;
  http.setTimeout(10000); // 10 sec timeout
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"data\":\"" + data + "\"}";
  int httpCode = http.POST(json);

  Serial.print("HTTP Code: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.print("Server Response: ");
    Serial.println(response);

    if (httpCode >= 200 && httpCode < 300) {
      Serial.println("✅ SUCCESS: Data sent");
      showStatus("SENT");
    } else {
      Serial.println("❌ SERVER ERROR");
      showStatus("SRV ERR");
    }
  }
  else {
    Serial.println("❌ HTTP FAILED");
    Serial.print("Reason: ");
    Serial.println(http.errorToString(httpCode));
    showStatus("HTTP ERR");
  }

  http.end();
}

// ================= HTTP GET WITH DEBUG =================
void getData(String type) {

  Serial.println("\n--- GET DATA ---");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ FAIL: WiFi DISCONNECTED");
    return;
  }

  HTTPClient http;
  String url = "https://fastworking.onrender.com/api/" + type;
  http.begin(url);

  int code = http.GET();
  Serial.print("HTTP Code: ");
  Serial.println(code);

  if (code > 0) {
    String payload = http.getString();
    Serial.println("Response:");
    Serial.println(payload);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println(type + ":");
    display.println(payload);
    display.display();
  } else {
    Serial.print("❌ GET FAILED: ");
    Serial.println(http.errorToString(code));
  }

  delay(5000);
  http.end();
}

// ================= OLED STATUS =================
void showStatus(const char* msg) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println(msg);
  display.display();
  delay(2000);
}