# ESP32 Transaction Server

Prototype Express.js server with MongoDB for handling transactions from ESP32 keypad.

## Setup

1. Install dependencies: `npm install`

2. Create `.env` file with `MONGO_URI=your_mongodb_uri`

3. Run: `npm start`

## APIs

- POST /api/transaction : { "data": "*101300" } for buying cash from 00 amount 300

- GET /api/sells : { "total_sells": 123 }

- GET /api/buys : { "total_buys": 456 }

## Entities (Customers for Selling)

00: rahim

01: karim

02: asif

03: jamal

04: nasir

05: faruk

06: salim

07: rahat

08: sakib

09: tamim

## Products (for Buying)

01: medribo

02: pran

03: merico

04: prans

05: product5

06: product6

07: product7

08: product8

09: product9

10: product10

## ESP32 Code

Use the `esp32_keypad.ino` file in Arduino IDE. Update WiFi credentials and server URL. Install Adafruit_SSD1306 library for OLED.