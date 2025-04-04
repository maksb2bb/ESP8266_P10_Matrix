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
const uint8_t *FONT = Font_BOLD;
const int WIDTH = 4;  // Количество матриц в ширину
const int HEIGHT = 3; // Количество матриц в высоту
const int DISPLAY_WIDTH = WIDTH * 32;
const int DISPLAY_HEIGHT = HEIGHT * 16;
const int TEXT_Y_OFFSET = 17;
const int SCROLL_DELAY = 5;  // Задержка для скролла в секундах
const int SCROLL_SPEED = 50; // Скорость скролла в мс

// Настройки WiFi
const char *SSID = "SKBKIT";
const char *PASSWORD = "skbkit2024";
IPAddress staticIP(10, 131, 170, 4);
IPAddress gateway(10, 131, 170, 1);
IPAddress subnet(255, 255, 255, 0);

// Текстовые константы
const char *SKB PROGMEM = "СКБ";
const char *KIT PROGMEM = "\"КИТ\"";
const char *ICTIB PROGMEM = "ИКТИБ";

// Перечисления для состояний
enum class PanelState : int
{
  STATIC_LOGO = 0,
  SCROLLING_TEXT = 1,
  ANIMATED_LOGO = 2,
  UNKNOWN = -1
};

enum class PanelPower : int
{
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
DMD_TextBox *box = nullptr;
Ticker modeTicker;
Ticker scrollTicker;

void setupWiFi()
{
  Serial.print("Подключение к WiFi ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASSWORD);
  if (!WiFi.config(staticIP, gateway, subnet))
  {
    Serial.println("Ошибка настройки статического IP!");
  }
  else
  {
    Serial.println("Статический IP настроен");
  }

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nПодключено к WiFi");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());
}

void drawBinaryArray(const uint8_t *data)
{
  dmd.clearScreen();

  for (int y = 0; y < DISPLAY_HEIGHT; y++)
  {
    for (int x = 0; x < DISPLAY_WIDTH; x++)
    {
      int idx = y * DISPLAY_WIDTH + x;
      dmd.setPixel(x, y, data[idx] ? GRAPHICS_ON : GRAPHICS_OFF);
    }
  }
}

void switchDisplayMode()
{
  if (panelPower == PanelPower::POWER_ON)
  {
    if (showText)
    {
      drawScrollingText();
    }
    else
    {
      drawBinaryArray(output);
    }
    showText = !showText;
  }
}

void handleScroll()
{
  if (panelPower == PanelPower::POWER_ON &&
      currentState == PanelState::SCROLLING_TEXT &&
      scrollEnabled)
  {
    dmd.marqueeScrollX(-1); // Используем доступный метод скроллинга
    scrollPosition--;

    // Если текст полностью прокрутился, сбрасываем позицию
    if (scrollPosition < -dmd.stringWidth(displayText, Font_BOLD))
    {
      scrollPosition = DISPLAY_WIDTH;
    }
  }
}

void drawScrollingText()
{
  if (!box)
  {
    box = new DMD_TextBox(dmd, 0, TEXT_Y_OFFSET, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  }
  else
  {
    box->clear();
  }
  box->print(displayText.c_str());

  scrollEnabled = (dmd.stringWidth(displayText, Font_BOLD) > DISPLAY_WIDTH);
  if (scrollEnabled)
  {
    scrollPosition = 0;
  }
}

void drawKitLogo()
{
  dmd.drawLine(0, 14, 16, 14, GRAPHICS_ON); // x y x y
  dmd.drawLine(0, 13, 16, 13, GRAPHICS_ON);

  dmd.drawLine(16, 14, 20, 5, GRAPHICS_ON);
  dmd.drawLine(16, 13, 20, 4, GRAPHICS_ON);

  dmd.drawLine(20, 5, 30, 5, GRAPHICS_ON);
  dmd.drawLine(20, 4, 30, 4, GRAPHICS_ON);

  dmd.drawCircle(33, 4, 3, GRAPHICS_ON);
  dmd.drawCircle(33, 4, 2, GRAPHICS_ON); // 1 line

  dmd.drawLine(20, 14, 50, 14, GRAPHICS_ON);
  dmd.drawLine(20, 13, 50, 13, GRAPHICS_ON);

  dmd.drawLine(50, 14, 55, 5, GRAPHICS_ON);
  dmd.drawLine(50, 13, 55, 4, GRAPHICS_ON);

  dmd.drawLine(55, 5, 70, 5, GRAPHICS_ON);
  dmd.drawLine(55, 4, 70, 4, GRAPHICS_ON);

  dmd.drawLine(70, 5, 75, 14, GRAPHICS_ON);
  dmd.drawLine(70, 4, 75, 13, GRAPHICS_ON);

  dmd.drawLine(75, 14, 85, 14, GRAPHICS_ON);
  dmd.drawLine(75, 13, 85, 13, GRAPHICS_ON);
  dmd.drawCircle(88, 13, 3, GRAPHICS_ON);
  dmd.drawCircle(88, 13, 2, GRAPHICS_ON); // 2 line

  dmd.drawCircle(78, 4, 3, GRAPHICS_ON);
  dmd.drawCircle(78, 4, 2, GRAPHICS_ON);
  dmd.drawLine(81, 5, 95, 5, GRAPHICS_ON);
  dmd.drawLine(81, 4, 95, 4, GRAPHICS_ON);
  dmd.drawLine(95, 5, 100, 14, GRAPHICS_ON);
  dmd.drawLine(95, 4, 100, 13, GRAPHICS_ON);
  dmd.drawLine(100, 14, 110, 14, GRAPHICS_ON);
  dmd.drawLine(100, 13, 110, 13, GRAPHICS_ON);
  dmd.drawLine(110, 14, 115, 5, GRAPHICS_ON);
  dmd.drawLine(110, 13, 115, 4, GRAPHICS_ON);
  dmd.drawLine(115, 5, 128, 5, GRAPHICS_ON);
  dmd.drawLine(115, 4, 128, 4, GRAPHICS_ON); // 3 line

  // ======================================

  dmd.drawLine(0, 35, 16, 35, GRAPHICS_ON);
  dmd.drawLine(0, 36, 16, 36, GRAPHICS_ON);
  dmd.drawLine(16, 35, 21, 43, GRAPHICS_ON);
  dmd.drawLine(16, 36, 21, 44, GRAPHICS_ON);
  dmd.drawLine(21, 43, 31, 43, GRAPHICS_ON);
  dmd.drawLine(21, 44, 31, 44, GRAPHICS_ON);
  dmd.drawCircle(34, 44, 3, GRAPHICS_ON);
  dmd.drawCircle(34, 44, 2, GRAPHICS_ON); // 1 line

  dmd.drawLine(20, 35, 50, 35, GRAPHICS_ON);
  dmd.drawLine(20, 36, 50, 36, GRAPHICS_ON);
  dmd.drawLine(50, 35, 55, 43, GRAPHICS_ON);
  dmd.drawLine(50, 36, 55, 44, GRAPHICS_ON); // 2 line

  dmd.drawLine(55, 43, 70, 43, GRAPHICS_ON);
  dmd.drawLine(55, 44, 70, 44, GRAPHICS_ON);

  dmd.drawLine(70, 43, 75, 35, GRAPHICS_ON);
  dmd.drawLine(70, 44, 75, 36, GRAPHICS_ON);
  dmd.drawLine(75, 35, 85, 35, GRAPHICS_ON);
  dmd.drawLine(75, 36, 85, 36, GRAPHICS_ON);
  dmd.drawCircle(88, 36, 3, GRAPHICS_ON);
  dmd.drawCircle(88, 36, 2, GRAPHICS_ON);

  dmd.drawCircle(78, 44, 3, GRAPHICS_ON);
  dmd.drawCircle(78, 44, 2, GRAPHICS_ON);
  dmd.drawLine(81, 43, 95, 43, GRAPHICS_ON);
  dmd.drawLine(81, 44, 95, 44, GRAPHICS_ON);
  dmd.drawLine(95, 43, 100, 35, GRAPHICS_ON);
  dmd.drawLine(95, 44, 100, 36, GRAPHICS_ON);
  dmd.drawLine(100, 35, 110, 35, GRAPHICS_ON);
  dmd.drawLine(100, 36, 110, 36, GRAPHICS_ON);
  dmd.drawLine(110, 35, 115, 43, GRAPHICS_ON);
  dmd.drawLine(110, 36, 115, 44, GRAPHICS_ON);
  dmd.drawLine(115, 43, 128, 43, GRAPHICS_ON);
  dmd.drawLine(115,44, 128, 44, GRAPHICS_ON);
}

void drawKitLeftLogoWithText()
{
  drawBinaryArray(kitleft);
  dmd.drawString_P(79, 2, SKB);
  dmd.drawString_P(71, 17, KIT);
  dmd.drawString_P(68, 32, ICTIB);
}

void updateScreen()
{
    modeTicker.detach();
    scrollTicker.detach();

  if (panelPower == PanelPower::POWER_OFF)
  {
    dmd.clearScreen();
    return;
  }

  modeTicker.detach();
  scrollTicker.detach();

  switch (currentState)
  {
  case PanelState::SCROLLING_TEXT:

    dmd.clearScreen();
    dmd.selectFont(FONT);
    drawText();
    drawKitLogo();

    scrollEnabled = (dmd.stringWidth(displayText, Font_BOLD) > DISPLAY_WIDTH);
    if (scrollEnabled)
    {
      scrollTicker.attach_ms(SCROLL_SPEED, handleScroll);
    }
    else
    {
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
    dmd.clearScreen();
    drawKitLeftLogoWithText();
    break;

  default:
    dmd.clearScreen();
    drawKitLeftLogoWithText();
    break;
  }
}

void drawText()
{
  int textWidth = dmd.stringWidth(displayText, Font_BOLD);
  int xOffset = (textWidth < DISPLAY_WIDTH) ? (DISPLAY_WIDTH - textWidth) / 2 : 0;

  if (box)
  {
    delete box;
    box = nullptr;
  }

  box = new DMD_TextBox(dmd, xOffset, TEXT_Y_OFFSET, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  box->print(displayText.c_str());
}

PanelState intToPanelState(int state)
{
  switch (state)
  {
  case 0:
    return PanelState::STATIC_LOGO;
  case 1:
    return PanelState::SCROLLING_TEXT;
  case 2:
    return PanelState::ANIMATED_LOGO;
  default:
    return PanelState::UNKNOWN;
  }
}

String panelStateToString(PanelState state)
{
  switch (state)
  {
  case PanelState::STATIC_LOGO:
    return "0";
  case PanelState::SCROLLING_TEXT:
    return "1";
  case PanelState::ANIMATED_LOGO:
    return "2";
  default:
    return "-1";
  }
}

void handleText(AsyncWebServerRequest *request, const JsonVariant &json)
{
  if (!json.containsKey("text"))
  {
    request->send(400, "application/json", "{\"error\": \"Missing text field\"}");
    return;
  }

  displayText = json["text"].as<String>();
  isUpdater = true;
  updateScreen();

  request->send(200, "application/json", "{\"status\": \"OK\"}");
}

void handleTurn(AsyncWebServerRequest* request, const JsonVariant& json) {
    bool powerChanged = false;
    bool modeChanged = false;

    // Обработка включения/выключения панели
    if (json.containsKey("panel")) {
        String panelTurn = json["panel"].as<String>();
        
        PanelPower newPower = (panelTurn == "om") ? PanelPower::POWER_ON : PanelPower::POWER_OFF;
        powerChanged = (panelPower != newPower);
        
        if (powerChanged) {
            panelPower = newPower;
        }
    }

    // Обработка изменения режима
    if (json.containsKey("state")) {
        int panelStateInt = json["state"].as<int>();
        
        PanelState newState = intToPanelState(panelStateInt);
        
        if (newState == PanelState::UNKNOWN) {
            request->send(400, "application/json", "{\"error\": \"Invalid state value\"}");
            return;
        }

        modeChanged = (currentState != newState);
        if (modeChanged) {
            currentState = newState;
        }
    }

    if (powerChanged || modeChanged) {
        updateScreen();
    } else {
    }

    String response = "{\"panel\":\"" + String(panelPower == PanelPower::POWER_ON ? "on" : "off") + 
                     "\",\"state\":" + String(static_cast<int>(currentState)) + "}";
    request->send(200, "application/json", response);
}

void setup()
{
  Serial.begin(9600);

  setupWiFi();

  dmd.setBrightness(255);
  dmd.selectFont(FONT);
  dmd.begin();
  dmd.clearScreen();

  panelPower = PanelPower::POWER_ON;
  updateScreen();

  server.on("/api/text", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
            {
      StaticJsonDocument<256> jsonReq;
      DeserializationError error = deserializeJson(jsonReq, data, len);

      if (error) {
        request->send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
        return;
      }

      handleText(request, jsonReq.as<JsonVariant>()); });

  server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "application/json",
                            String("{\"panel\":\"") + (panelPower == PanelPower::POWER_ON ? "on" : "off") +
                                "\",\"state\":" + String(static_cast<int>(currentState)) + "}"); });

  server.on("/api/led", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t)
            {
      StaticJsonDocument<256> jsonReq;
      DeserializationError error = deserializeJson(jsonReq, data, len);

      if (error) {
        request->send(400, "application/json", "{\"error\": \"Invalid JSON\"}");
        return;
      }

      handleTurn(request, jsonReq.as<JsonVariant>()); });

  server.begin();
}

void loop()
{
  if (isUpdater)
  {
    isUpdater = false;
    updateScreen();
  }
  if (scrollEnabled && currentState == PanelState::SCROLLING_TEXT)
  {
    dmd.marqueeScrollX(-1);
    delay(SCROLL_SPEED);
  }
}
