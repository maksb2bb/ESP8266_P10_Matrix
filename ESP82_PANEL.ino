#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <DMD2.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>
#include <fonts/Font_BOLD.h>
#include "data/output.h"
#include "data/kitleft.h"

// Константы
const uint8_t* FONT = Font_BOLD;
const int WIDTH = 4;   // Количество матриц в ширину
const int HEIGHT = 3;  // Количество матриц в высоту
const int DISPLAY_WIDTH = WIDTH * 32;
const int DISPLAY_HEIGHT = HEIGHT * 16;
const int TEXT_Y_OFFSET = 17;
const int SCROLL_DELAY = 5; // Задержка для скролла в секундах
const int SCROLL_SPEED = 50; // Скорость скролла в мс

// Настройки WiFi
const char* SSID = "YOUR_SSID"; //Updete this
const char* PASSWORD = "YOUR_PASSWORD"; //Update this
IPAddress staticIP(10, 131, 170, 4);
IPAddress gateway(10, 131, 170, 1);
IPAddress subnet(255, 255, 255, 0);

// Текстовые константы
const char* SKB PROGMEM = "СКБ";
const char* KIT PROGMEM = "\"КИТ\"";
const char* ICTIB PROGMEM = "ИКТИБ";

// Перечисления для состояний
enum class PanelState : int {
  STATIC_LOGO = 0,
  SCROLLING_TEXT = 1,
  ANIMATED_LOGO = 2,
  UNKNOWN = -1
};

enum class PanelPower : int {
  POWER_OFF = 0,
  POWER_ON = 1
};

// Глобальные переменные
AsyncWebServer server(80);
String displayText = "Привет из СКБ \"КИТ\"";
PanelState currentState = PanelState::STATIC_LOGO;
PanelPower panelPower = PanelPower::POWER_ON;
bool showText = true;
bool isUpdater = false;
bool scrollEnabled = false;
int scrollPosition = 0;

SPIDMD dmd(WIDTH, HEIGHT);
DMD_TextBox* box = nullptr;
Ticker modeTicker;
Ticker scrollTicker;

void setupWiFi() {
  Serial.print("Подключение к WiFi ");
  Serial.println(SSID);
  
  WiFi.begin(SSID, PASSWORD);
  if (!WiFi.config(staticIP, gateway, subnet)) {
    Serial.println("Ошибка настройки статического IP!");
  } else {
    Serial.println("Статический IP настроен");
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nПодключено к WiFi");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());
}

void drawBinaryArray(const uint8_t* data) {
  dmd.clearScreen();
  
  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      int idx = y * DISPLAY_WIDTH + x;
      dmd.setPixel(x, y, data[idx] ? GRAPHICS_ON : GRAPHICS_OFF);
    }
  }
}

int calculateTextWidth(const String &text) {
  int totalWidth = 0;
  for (unsigned int i = 0; i < text.length(); i++) {
    totalWidth += dmd.charWidth(text[i]);
  }
  return totalWidth;
}

void switchDisplayMode() {
  if (panelPower == PanelPower::POWER_ON) {
    if (showText) {
      drawScrollingText();
    } else {
      drawBinaryArray(output);
    }
    showText = !showText;
  }
}

void handleScroll() {
  if (panelPower == PanelPower::POWER_ON && 
      currentState == PanelState::SCROLLING_TEXT && 
      scrollEnabled) {
    dmd.marqueeScrollX(-1); // Используем доступный метод скроллинга
    scrollPosition--;
    
    // Если текст полностью прокрутился, сбрасываем позицию
    if (scrollPosition < -calculateTextWidth(displayText)) {
      scrollPosition = DISPLAY_WIDTH;
    }
  }
}

void drawScrollingText() {
  if (!box) {
    box = new DMD_TextBox(dmd, 0, TEXT_Y_OFFSET, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  } else {
    box->clear();
  }
  box->print(displayText.c_str());
  
  scrollEnabled = (calculateTextWidth(displayText) > DISPLAY_WIDTH);
  if (scrollEnabled) {
    scrollPosition = 0;
  }
}

void drawKitLogo() {
  // Верхняя часть логотипа
  dmd.drawLine(0, 14, 16, 14, GRAPHICS_ON);
  dmd.drawLine(0, 13, 16, 13, GRAPHICS_ON);
  dmd.drawLine(16, 14, 20, 5, GRAPHICS_ON);
  dmd.drawLine(16, 13, 20, 4, GRAPHICS_ON);
  dmd.drawLine(20, 5, 30, 5, GRAPHICS_ON);
  dmd.drawLine(20, 4, 30, 4, GRAPHICS_ON);
  dmd.drawCircle(33, 4, 3, GRAPHICS_ON);
  dmd.drawCircle(33, 4, 2, GRAPHICS_ON);

  // Средняя часть логотипа
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
  dmd.drawCircle(78, 13, 2, GRAPHICS_ON);

  // Нижняя часть логотипа
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
  dmd.drawLine(105, 4, 128, 4, GRAPHICS_ON);
}

void drawKitLeftLogoWithText() {
  drawBinaryArray(kitleft);
  dmd.drawString_P(79, 2, SKB);
  dmd.drawString_P(71, 17, KIT);
  dmd.drawString_P(68, 32, ICTIB);
}

void updateScreen() {
  if (panelPower == PanelPower::POWER_OFF) {
    dmd.clearScreen();
    modeTicker.detach();
    scrollTicker.detach();
    return;
  }

  modeTicker.detach();
  scrollTicker.detach();

  switch (currentState) {
    case PanelState::SCROLLING_TEXT:
      
      dmd.clearScreen();
      dmd.selectFont(FONT);
      drawText();
      drawKitLogo();
      
      scrollEnabled = (calculateTextWidth(displayText) > DISPLAY_WIDTH);
      if (scrollEnabled) {
        scrollTicker.attach_ms(SCROLL_SPEED, handleScroll);
      } else {
        Serial.println("Текст помещается - скроллинг не требуется");
      }
      break;
      
    case PanelState::ANIMATED_LOGO:
      dmd.clearScreen();
      drawText();
      modeTicker.attach(SCROLL_DELAY, switchDisplayMode);
      dmd.clearScreen();
      drawBinaryArray(output);
      break;
      
    case PanelState::STATIC_LOGO:
      Serial.println("Активация режима: STATIC_LOGO");
      dmd.clearScreen();
      drawKitLeftLogoWithText();
      break;
      
    default:
      dmd.clearScreen();
      drawKitLeftLogoWithText();
      break;
  }
}

void drawText() {
  int textWidth = calculateTextWidth(displayText);
  int xOffset = (textWidth < DISPLAY_WIDTH) ? (DISPLAY_WIDTH - textWidth) / 2 : 0;

  if (box) {
    delete box;
    box = nullptr;
  }

  box = new DMD_TextBox(dmd, xOffset, TEXT_Y_OFFSET, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  box->print(displayText.c_str());
}

PanelState intToPanelState(int state) {
  switch (state) {
    case 0: return PanelState::STATIC_LOGO;
    case 1: return PanelState::SCROLLING_TEXT;
    case 2: return PanelState::ANIMATED_LOGO;
    default: return PanelState::UNKNOWN;
  }
}

String panelStateToString(PanelState state) {
  switch (state) {
    case PanelState::STATIC_LOGO: return "0";
    case PanelState::SCROLLING_TEXT: return "1";
    case PanelState::ANIMATED_LOGO: return "2";
    default: return "-1";
  }
}

void handleText(AsyncWebServerRequest* request, const JsonVariant& json) {
  if (!json.containsKey("text")) {
    Serial.println("Ошибка: запрос text без поля 'text'");
    request->send(400, "application/json", "{\"error\": \"Missing text field\"}");
    return;
  }

  displayText = json["text"].as<String>();
  Serial.print("Получен новый текст: '"); Serial.print(displayText); Serial.println("'");
  isUpdater = true;
  updateScreen();
  
  request->send(200, "application/json", "{\"status\": \"OK\"}");
}

void handleTurn(AsyncWebServerRequest* request, const JsonVariant& json) {
  if (!json.containsKey("state")) {
    Serial.println("Ошибка: запрос led без поля 'state'");
    request->send(400, "application/json", "{\"error\": \"Missing state field\"}");
    return;
  }

  int panelStateInt = json["state"].as<int>();
  PanelState newState = intToPanelState(panelStateInt);
  
  if (newState == PanelState::UNKNOWN) {
    Serial.print("Ошибка: недопустимое состояние: "); Serial.println(panelStateInt);
    request->send(400, "application/json", "{\"error\": \"Invalid state value\"}");
    return;
  }

  modeTicker.detach();
  scrollTicker.detach();

  currentState = newState;

  if (json.containsKey("panel")) {
    String panelTurn = json["panel"].as<String>();
    panelPower = (panelTurn == "on") ? PanelPower::POWER_ON : PanelPower::POWER_OFF;
  }

  isUpdater = true;
  updateScreen();

  String response = "{\"panel\":\"" + String(panelPower == PanelPower::POWER_ON ? "on" : "off") + 
                   "\",\"state\":" + String(static_cast<int>(currentState)) + "}";
  request->send(200, "application/json", response);
}

void setup() {
  Serial.begin(9600);
  
  setupWiFi();
  
  dmd.setBrightness(255);
  dmd.selectFont(FONT);
  dmd.begin();
  dmd.clearScreen();

  panelPower = PanelPower::POWER_ON;
  Serial.println("Стартовый режим: STATIC_LOGO (0)");
  updateScreen();

  server.on("/api/text", HTTP_POST, 
    [](AsyncWebServerRequest* request) {},
    NULL,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
      StaticJsonDocument<256> jsonReq;
      DeserializationError error = deserializeJson(jsonReq, data, len);

      if (error) {
        Serial.print("Ошибка парсинга JSON: "); Serial.println(error.c_str());
        request->send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
        return;
      }

      handleText(request, jsonReq.as<JsonVariant>());
    });

  server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("Запрос текущего состояния");
    request->send(200, "application/json", 
      String("{\"panel\":\"") + (panelPower == PanelPower::POWER_ON ? "on" : "off") + 
      "\",\"state\":" + String(static_cast<int>(currentState)) + "}");
  });

  server.on("/api/led", HTTP_POST, 
    [](AsyncWebServerRequest* request) {},
    NULL,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
      StaticJsonDocument<256> jsonReq;
      DeserializationError error = deserializeJson(jsonReq, data, len);

      if (error) {
        Serial.print("Ошибка парсинга JSON: "); Serial.println(error.c_str());
        request->send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
        return;
      }

      handleTurn(request, jsonReq.as<JsonVariant>());
    });

  server.begin();
}

void loop() {
  if (isUpdater) {
    isUpdater = false;
    updateScreen();
  }
}
