//----------------include libraries----------------//
#include <WiFi.h>
#include <WebServer.h>
#include <DMD32.h>
#include <fonts/Font_BOLD.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
//-------------------------------------------------//

//----------------settings panel----------------//
#define FONT Font_BOLD     //Default font
#define DISPLAYS_ACROSS 1  //number of panels in width
#define DISPLAYS_DOWN 1    //number of panels in height
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
//----------------------------------------------//

//----------------settings local wifi----------------//
const char *ssid = "SKBKIT";          //edit for your local wifi
const char *password = "skbkit2024";  //edit for your local wifi
//---------------------------------------------------//

//----------------settings web server----------------//
AsyncWebServer server(80);
IPAddress staticIP(10, 131, 170, 4);  //edit for your local ip
IPAddress gateway(10, 131, 170, 1);
IPAddress subnet(255, 255, 255, 0);
//---------------------------------------------------//

//----------------other variables----------------//
String displayText = "Привет из СКБ \"КИТ\"";  //change the default label
hw_timer_t *timer = NULL;
bool panel = true;
//-----------------------------------------------//

//---Search SPI panel---//
void IRAM_ATTR triggerScan() {
  dmd.scanDisplayBySPI();
}
//----------------------//

void reader() {}

void setup() {
  Serial.begin(115200);
  dmd.selectFont(Font_BOLD);
  WiFi.begin(ssid, password);
  SPIFFS.begin(true);
  pinMode(22, OUTPUT);

  //---Configured WebServer---//
  if (!WiFi.config(staticIP, gateway, subnet)) {
    Serial.println("Failed to configure Static IP");
  } else {
    Serial.println("Static IP configured!");
  }
  //--------------------------//

  //---Start file system---//
  // File file = SPIFFS.open("/output.txt", "r");
  // if(!file){Serial.println("file open failed");}
  // Serial.println("Содержимое файла:");
  // while(file.available()){
  //   Serial.write(file.read());
  // }
  // Serial.println();
  // file.close();

  // File root = SPIFFS.open("/");
  // if(!root){
  //   Serial.println("Ошибка открытия директории");
  //   return;
  // }
  // if(!root.isDirectory()){
  //   Serial.println("Не является директорией!");
  //   return;
  // }

  // File file = root.openNextFile();
  // while(file){
  //   Serial.println(file.name());
  //   file = root.openNextFile();
  //}
  //-----------------------//

  //---Start the WiFi---//
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());
  //--------------------//

  //---Request to check the status of the panel---//
  server.on("/api/led", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", panel ? "{\"state\": \"true\"}" : "{\"state\": \"false\"}");
  });
  //----------------------------------------------//

  //---Request to change the status of the panel---//
  /*server.on("/api/led", HTTP_POST, 
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
  {
    AsyncWebServerRequest *req = request;
    Serial.println(String("data=") + (char*)data);
    // // Serial.print(request);

    // if (req->hasParam("plain", true)) {
    //   String panelState = req->getParam("panel", true)->value();
    //   Serial.println(panelState);

    //   if (panelState == "on") {
    //     Serial.println("Panel on!");
    //     displayText = "";
    //     panel = true;
    //   } else if (panelState == "off") {
        // panel = false;
        // Serial.println("Panel off!");
        // digitalWrite(22, LOW);
        // dmd.clearScreen(1);
        // displayText = "";
        // Serial.println("Screen clear");
    //   }
    // }
    req->send(200, "text/html", "");
  });*/

  server.on(
    "/api/led", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      // Serial.println("1");
      // Serial.println(request->url() );
      // Serial.println(request->method());
    },
    [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
      //Serial.println("2");
    },
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String dataRequest = String((char *)data);
        Serial.println(dataRequest);
        if (dataRequest == "{\"panel\" : \"on\"}") {
          Serial.println("Panel on");
        } else if (dataRequest == "{\"panel\" : \"off\"}") {
          panel = false;
          Serial.println("Panel off!");
          digitalWrite(22, LOW);
          dmd.clearScreen(1);
          displayText = "";
          Serial.println("Screen clear");
        }
        request->send(200, "text/html", "");
    });
  //-----------------------------------------------//

  //---A text modification request---//
  server.on("/api/text", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text", true)) {
      displayText = request->getParam("text", true)->value();
    }
    request->send(200, "text/html; charset=UTF-8",
                  "<html><body><h2>Text set to:</h2><p>" + displayText + "</p></body></html>");
  });
  //---------------------------------//

  server.begin();

  //---Start the timer---//
  uint8_t cpuClock = ESP.getCpuFreqMHz();
  timer = timerBegin(0, cpuClock, true);
  timerAttachInterrupt(timer, &triggerScan, true);
  timerAlarmWrite(timer, 300, true);
  timerAlarmEnable(timer);
  //--------------------//

  dmd.clearScreen(true);  //Clearing the screen
}

void loop() {

  //---Screen cleaning and text output---//
  dmd.clearScreen(true);
  dmd.drawMarquee(displayText.c_str(), displayText.length(), (32 * DISPLAYS_ACROSS) - 1, 0);
  //-------------------------------------//

  long start = millis();
  long timer = start;
  boolean ret = false;

  //---Scrolling text---//
  while (!ret) {
    if ((timer + 30) < millis()) {
      ret = dmd.stepMarquee(-1, 0);  // Прокрутка текста
      timer = millis();
    }
  }
  //--------------------//
}
