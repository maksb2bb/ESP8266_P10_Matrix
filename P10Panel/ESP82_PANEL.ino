#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <DMD2.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>
#include <fonts/Font_BOLD.h>
#include "data/output.h"
#include "data/kitleft.h"

const uint8_t* FONT = Font_BOLD;
const int WIDTH = 4;   // Количество матриц в ширину
const int HEIGHT = 3;  // Количество матриц в высоту

AsyncWebServer server(80);
String displayText = "Привет из СКБ \"КИТ\"";
bool isON = true;
bool showText = true;
bool scroll = true;
const char* skb = "СКБ";
const char* kit = "\"КИТ\"";
const char* ictib = "ИКТИБ";
const char* ssid = "SKBKIT";
const char* password = "skbkit2024";
IPAddress staticIP(10, 131, 170, 4);
IPAddress gateway(10, 131, 170, 1);
IPAddress subnet(255, 255, 255, 0);

SPIDMD dmd(WIDTH, HEIGHT);
DMD_TextBox* box = nullptr;  // Указатель на текстовый бокс
Ticker ticker;

void setupWiFi() {
  WiFi.begin(ssid, password);
  if (!WiFi.config(staticIP, gateway, subnet)) {
    Serial.println("Failed WiFi connect!");
  } else {
    Serial.println("Static WiFi configured!");
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());
}

void drawBinaryArrayKit() {
  dmd.clearScreen();

  for (int x = 0; x < WIDTH * 32; x++) {
    for (int y = 0; y < HEIGHT * 16; y++) {
      int idx = y * (WIDTH * 32) + x;  // Индекс с учетом полной ширины
      if (output[idx] == 1) {
        dmd.setPixel(x, y, GRAPHICS_ON);
      } else {
        dmd.setPixel(x, y, GRAPHICS_OFF);
      }
    }
  }
}

void drawBinaryArrayKitLeft() {
  dmd.clearScreen();

  for (int x = 0; x < WIDTH * 32; x++) {
    for (int y = 0; y < HEIGHT * 16; y++) {
      int idx = y * (WIDTH * 32) + x;  // Индекс с учетом полной ширины
      if (kitleft[idx] == 1) {
        dmd.setPixel(x, y, GRAPHICS_ON);
      } else {
        dmd.setPixel(x, y, GRAPHICS_OFF);
      }
    }
  }
}

void switchDisplayMode() {
  if (isON) {
    if (showText) {
      drawScrollingText();  // Запускаем скролл текста на второй строке
    } else {
      drawBinaryArrayKit();
    }
    showText = !showText;
  }
}

void switchDisplayModeScroll() {
  if (isON && scroll) {
    //box->scrollX(1); // Двигаем текст влево
    //box->drawScrollingText();
    dmd.marqueeScrollX(1);
  }
}

void drawScrollingText() {
  if (!box) {
    box = new DMD_TextBox(dmd, 0, 17, WIDTH * 32, HEIGHT * 16);  // Устанавливаем текстовый бокс на вторую строку
  } else {
    box->clear();
  }

  box->print(displayText.c_str());
}



void drawText() {
  dmd.clearScreen();

  if (!box) {  // Создаём текстовый бокс только один раз
    box = new DMD_TextBox(dmd, 0, 0, WIDTH * 32, HEIGHT * 16);
  } else {
    box->clear();  // Очищаем текущий бокс перед обновлением текста
  }

  box->print(displayText.c_str());
}

void handleText(AsyncWebServerRequest* request, const JsonVariant& json) {
  if (!json.containsKey("text")) {
    request->send(400, "application/json", "{\"error\": \"Missing text field\"}");
    return;
  }

  displayText = json["text"].as<String>();
  drawText();

  request->send(200, "application/json", "{\"status\": \"OK\"}");
}

void handleTurn(AsyncWebServerRequest* request, const JsonVariant& json) {
  String panelTurn = json["panel"].as<String>();
  String panelState = json["state"].as<String>();

  if (json.containsKey("panel")) {
    panelTurn = json["panel"].as<String>();
  }
  if (json.containsKey("state")) {
    panelState = json["state"].as<String>();
  }

  ticker.detach();

  if (!panelTurn.isEmpty()) {
    if (panelTurn == "on") {
      isON = true;
    } else if (panelTurn == "off") {
      isON = false;
      dmd.clearScreen();
      dmd.drawFilledBox(0, 0, WIDTH * 32 - 1, HEIGHT * 16 - 1, GRAPHICS_OFF);
    }
  }

  if (!panelState.isEmpty()) {
    if (panelState == "1") {  // Скролинг текста
      dmd.clearScreen();
      dmd.selectFont(FONT);
      dmd.drawLine(0, 14, 16, 14, GRAPHICS_ON);  // x y x y
      dmd.drawLine(0, 13, 16, 13, GRAPHICS_ON);

      dmd.drawLine(16, 14, 20, 5, GRAPHICS_ON);
      dmd.drawLine(16, 13, 20, 4, GRAPHICS_ON);

      dmd.drawLine(20, 5, 30, 5, GRAPHICS_ON);
      dmd.drawLine(20, 4, 30, 4, GRAPHICS_ON);

      dmd.drawCircle(33, 4, 3, GRAPHICS_ON);
      dmd.drawCircle(33, 4, 2, GRAPHICS_ON);  //1 line

      dmd.drawLine(20, 14, 40, 14, GRAPHICS_ON);
      dmd.drawLine(20, 13, 40, 13, GRAPHICS_ON);

      dmd.drawLine(40, 14, 45, 5, GRAPHICS_ON);
      dmd.drawLine(40, 13, 45, 4, GRAPHICS_ON);

      dmd.drawLine(45, 5, 60, 5, GRAPHICS_ON);
      dmd.drawLine(45, 4, 60, 4, GRAPHICS_ON);

      dmd.drawLine(60, 5, 65, 14, GRAPHICS_ON);
      dmd.drawLine(60, 4, 65, 13, GRAPHICS_ON);

      dmd.drawLine(65, 14, 75, 14, GRAPHICS_ON);
      dmd.drawLine(65, 13, 75, 13, GRAPHICS_ON);
      dmd.drawCircle(78, 13, 3, GRAPHICS_ON);
      dmd.drawCircle(78, 13, 2, GRAPHICS_ON);  //2 line

      dmd.drawCircle(68, 4, 3, GRAPHICS_ON);
      dmd.drawCircle(68, 4, 2, GRAPHICS_ON);
      dmd.drawLine(71, 5, 85, 5, GRAPHICS_ON);
      dmd.drawLine(71, 4, 85, 4, GRAPHICS_ON);
      dmd.drawLine(85, 5, 90, 14, GRAPHICS_ON);
      dmd.drawLine(85, 4, 90, 13, GRAPHICS_ON);
      dmd.drawLine(90, 14, 100, 14, GRAPHICS_ON);
      dmd.drawLine(90, 13, 100, 13, GRAPHICS_ON);
      dmd.drawLine(100, 14, 105, 5, GRAPHICS_ON);
      dmd.drawLine(100, 13, 105, 4, GRAPHICS_ON);
      dmd.drawLine(105, 5, 128, 5, GRAPHICS_ON);
      dmd.drawLine(105, 4, 128, 4, GRAPHICS_ON);  // 3 line

      // ======================================

      dmd.drawLine(0, 35, 16, 35, GRAPHICS_ON);
      dmd.drawLine(0, 36, 16, 36, GRAPHICS_ON);
      dmd.drawLine(16, 35, 21, 43, GRAPHICS_ON);
      dmd.drawLine(16, 36, 21, 44, GRAPHICS_ON);
      dmd.drawLine(21, 43, 31, 43, GRAPHICS_ON);
      dmd.drawLine(21, 44, 31, 44, GRAPHICS_ON);
      dmd.drawCircle(34, 44, 3, GRAPHICS_ON);
      dmd.drawCircle(34, 44, 2, GRAPHICS_ON);  // 1 line

      dmd.drawLine(20, 35, 40, 35, GRAPHICS_ON);
      dmd.drawLine(20, 36, 40, 36, GRAPHICS_ON);
      dmd.drawLine(40, 35, 45, 43, GRAPHICS_ON);
      dmd.drawLine(40, 36, 45, 44, GRAPHICS_ON);  // 2 line

      dmd.drawLine(45, 43, 60, 43, GRAPHICS_ON);
      dmd.drawLine(45, 44, 60, 44, GRAPHICS_ON);

      dmd.drawLine(60, 43, 65, 35, GRAPHICS_ON);
      dmd.drawLine(60, 44, 65, 36, GRAPHICS_ON);
      dmd.drawLine(65, 35, 75, 35, GRAPHICS_ON);
      dmd.drawLine(65, 36, 75, 36, GRAPHICS_ON);
      dmd.drawCircle(78, 36, 3, GRAPHICS_ON);
      dmd.drawCircle(78, 36, 2, GRAPHICS_ON);
    } else if (panelState == "2") {  // Текст и картинка динамика
      dmd.clearScreen();
      drawText();
      ticker.attach(5, switchDisplayMode);
      dmd.clearScreen();
      drawBinaryArrayKit();
    } else if (panelState == "3") {  // Текст и картинка статика
      dmd.clearScreen();
      drawBinaryArrayKitLeft();
      dmd.drawString_P(79, 2, skb);
      dmd.drawString_P(71, 17, kit);
      dmd.drawString_P(68, 32, ictib);
    }
  }

  String response = "{\"panel\" : " + String(isON ? "\"on\"" : "\"off\"") + ", \"state\" : " + (panelState.isEmpty() ? "null" : panelState) + "}";
  request->send(200, "application/json", response);
}

void setup() {
  Serial.begin(9600);
  setupWiFi();
  dmd.setBrightness(255);
  dmd.selectFont(FONT);
  dmd.begin();
  dmd.clearScreen();

  box = new DMD_TextBox(dmd, 0, 0, WIDTH * 32, HEIGHT * 16);  // Создаём текстовый бокс один раз

  box->print("Hello");

  server.on(
    "/api/text", HTTP_POST,
    [](AsyncWebServerRequest* request) {},
    NULL,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
      StaticJsonDocument<256> jsonReq;
      DeserializationError error = deserializeJson(jsonReq, data, len);

      if (error) {
        request->send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
        return;
      }

      handleText(request, jsonReq.as<JsonVariant>());  // Передаём как const JsonVariant&
    });

  server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (isON) {
      request->send(200, "application/json", "{\"panel\" : \"on\"}");
    } else if (!isON) {
      request->send(200, "application/json", "{\"panel\" : \"off\"}");
    }
  });

  server.on(
    "/api/led", HTTP_POST, [](AsyncWebServerRequest* request) {},
    NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
      StaticJsonDocument<256> jsonReq;
      DeserializationError error = deserializeJson(jsonReq, data, len);

      if (error) {
        request->send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
        return;
      }

      handleTurn(request, jsonReq.as<JsonVariant>());
    });

  server.begin();
}

void loop() {
}
