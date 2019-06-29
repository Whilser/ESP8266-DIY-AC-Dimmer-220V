
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <RBDdimmer.h>
#include <Timer.h>
#include <uptime_formatter.h>

#define VERSION "0.1.1"
#define HARDWARE "ZCACD1"

#define LED_PIN D6
#define AC_LOAD D8
#define ZC_PIN D1
#define ONE_WIRE_BUS D5

WiFiClient WiFiEspClient;
WiFiServer server(2000);
WiFiUDP Udp;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
dimmerLamp dimmer(AC_LOAD, ZC_PIN);

struct ESPconfig {
  char SSID[32];
  char PASSWD[32];
};

ESPconfig readConfig () {
  int eeAddress = 0;
  ESPconfig wirelessConfig;

  EEPROM.begin(512);

  for (int i = 0; i < 32; ++i) wirelessConfig.SSID[i]   = char(EEPROM.read(eeAddress + i)); eeAddress = 32;
  for (int i = 0; i < 32; ++i) wirelessConfig.PASSWD[i] = char(EEPROM.read(eeAddress + i));

  EEPROM.end();
  return wirelessConfig;
}

bool writeConfig(const char SSID[32], const char PASSWD[32]) {
  int eeAddress = 0;

  EEPROM.begin(512);

  for (int i = 0; i < 512; i++) EEPROM.write(i, 0); // wipe eeprom
  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, SSID[i]); eeAddress = 32;
  for (int i = 0; i < 32; ++i)  EEPROM.write(eeAddress + i, PASSWD[i]);

  EEPROM.commit();
  EEPROM.end();

  return true;
}

void emergencyCallback() {
  sensors.requestTemperatures();
  float temperature = sensors.getTempC(insideThermometer);

  if (temperature > 40) {
    dimmer.setState(OFF);
    digitalWrite(LED_PIN, LOW);
    Serial.printf("Temperature is %.2f degrees celsius. Swith OFF on emergency reason.\n\r", temperature);
  } else Serial.printf("Temperature is %.2f degrees celsius\n\r", temperature);

}

Timer *emergencyTimer = new Timer(60000);

void setup() {

  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  dimmer.begin(NORMAL_MODE, OFF);
  sensors.begin();

  if (!sensors.getAddress(insideThermometer, 0)) {
    Serial.println("Thermal sensor is not installed.");
  }

  ESPconfig wirelessConfig = readConfig();

  WiFi.mode(WIFI_STA);
  WiFi.begin(wirelessConfig.SSID, wirelessConfig.PASSWD);
  Serial.printf("\nConnecting to %s ...", wirelessConfig.SSID);

  int timeOut = 0;
  while ((WiFi.status() != WL_CONNECTED) & (timeOut < 60)) {
      delay(500); timeOut++;
      Serial.print(".");
      digitalWrite(LED_PIN, not digitalRead(LED_PIN));

  }

  if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(LED_PIN, LOW);
      Serial.print("connection established! IP Address: ");
      Serial.println(WiFi.localIP());

  } else {
      Serial.printf("unable to connect to %s, starting software AP mode.\n", wirelessConfig.SSID);

      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP"+ESP.getChipId());
      Serial.printf("IP Address of software AP is %s\n", WiFi.softAPIP().toString().c_str());
  }

  server.begin(); Udp.begin(1000);

  emergencyTimer->setOnTimer(&emergencyCallback);
  emergencyTimer->Start();

  Serial.printf("DIY AC dimmer 220V version %s started.\n\r", VERSION);

}

String switchOn (int id) {

  dimmer.setState(ON);
  digitalWrite(LED_PIN, HIGH);

  StaticJsonDocument<200> jsonResult;
  jsonResult["id"] = id;
  dimmer.getState() ? jsonResult["state"] = "ON" : jsonResult["state"] = "OFF";

  String jsonReply;
  serializeJson(jsonResult, jsonReply);
  return jsonReply;

}

String switchOff (int id) {

  dimmer.setState(OFF);
  digitalWrite(LED_PIN, LOW);

  StaticJsonDocument<200> jsonResult;
  jsonResult["id"] = id;
  dimmer.getState() ? jsonResult["state"] = "ON" : jsonResult["state"] = "OFF";

  String jsonReply;
  serializeJson(jsonResult, jsonReply);
  return jsonReply;

}

String dimmTo (int id, int power, const char* state) {

  dimmer.setPower(power);
  if (String(state) == "ON") {
    dimmer.setState(ON); digitalWrite(LED_PIN, HIGH);

  } else {
    dimmer.setState(OFF); digitalWrite(LED_PIN, LOW);
  }

  StaticJsonDocument<200> jsonResult;
  jsonResult["id"] = id;
  jsonResult["power"] = dimmer.getPower();
  dimmer.getState() ? jsonResult["state"] = "ON" : jsonResult["state"] = "OFF";

  String jsonReply;
  serializeJson(jsonResult, jsonReply);
  return jsonReply;

}

// {"id":1, "method":"set_power", "power":"50", "state":"ON"}
// {"id":1, "method":"set_power", "power":"50", "state":"OFF"}
// {"id":1, "method":"set_state", "state":"OFF"}
// {"id":1, "method":"set_state", "state":"ON"}
// {"id":1, "method":"set_config", "SSID":"Wi-Fi SSID", "PASSWD": "PASSWORD"}
// {"id":1, "method":"get_temperature"}
// {"id":1, "method":"get_state"}
// {"id":1, "method":"update", "IP":"192.168.4.1", "url":"/update/firmware.bin"}
// {"id":1, "method":"set_mode", "mode":"TOGGLE_MODE"}

void discoverResponder() {

    if (Udp.parsePacket()) {
      char packetBuffer[255] = "";
      if (Udp.read(packetBuffer, 255) == 0) return;
      Serial.printf("Received broadcast message: %s from %s\n", packetBuffer, Udp.remoteIP().toString().c_str());

      if (String(packetBuffer) == "discover") {
        Udp.beginPacket(Udp.remoteIP(),Udp.remotePort());
        StaticJsonDocument<100> jsonResult;

        jsonResult["deviceID"] = String(ESP.getChipId(), HEX);
        WiFi.localIP() ? jsonResult["IP"] = WiFi.localIP().toString() : jsonResult["IP"] = WiFi.softAPIP().toString();
        jsonResult["hardware"] = HARDWARE;

        String jsonReply;
        serializeJson(jsonResult, jsonReply);

        Udp.print(jsonReply);
        Udp.endPacket();
      }
    }
}

void loop() {
  emergencyTimer->Update();

  discoverResponder();
  WiFiClient client = server.available();

  if (client) {

    if (client.connected()) Serial.println("Client connected");

    String json;
    while(client.connected()) {
        // read data from the connected client
        yield(); while(client.available()>0) json += char(client.read());

        if (json.length() != 0) {
            Serial.println("Received json command: " + json);

            StaticJsonDocument<200> root;
            auto error = deserializeJson(root, json);

            if ( (error) ||  (!root.containsKey("method")) || (!root.containsKey("id")) ) {
              StaticJsonDocument<200> jsonResult;

              jsonResult["id"] = root["id"];
              error ? jsonResult["result"] = error.c_str() : jsonResult["result"] = "InvalidInput";

              String jsonReply;
              serializeJson(jsonResult, jsonReply);
              client.println(jsonReply);
            }

            else {

              if ((root["method"] == "set_state") & root.containsKey("state")) {
                  if (root["state"] == "ON") client.println(switchOn(root["id"])); else client.println(switchOff(root["id"]));
              }

              if ((root["method"] == "set_power") & (root.containsKey("power") & root.containsKey("state"))) {
                client.println(dimmTo(root["id"], root["power"], root["state"]));
              }

              if (root["method"] == "set_config") {
                if (root.containsKey("SSID") & root.containsKey("PASSWD")) {
                  writeConfig(root["SSID"], root["PASSWD"]);
                  StaticJsonDocument<200> jsonResult;

                  jsonResult["id"] = root["id"];
                  jsonResult["result"] = "OK";

                  String jsonReply;
                  serializeJson(jsonResult, jsonReply);
                  client.println(jsonReply);
                  ESP.restart();
                }
              }

              if (root["method"] == "get_temperature") {
                StaticJsonDocument<200> jsonResult;

                jsonResult["id"] = root["id"];
                sensors.requestTemperatures();
                jsonResult["temperature"] = round(sensors.getTempC(insideThermometer)*10)/10;

                String jsonReply;
                serializeJson(jsonResult, jsonReply);
                client.println(jsonReply);
              }

              if (root["method"] == "get_state") {
                StaticJsonDocument<200> jsonResult;

                jsonResult["id"] = root["id"];
                jsonResult["power"] = dimmer.getPower();
                jsonResult["uptime"] = uptime_formatter::getUptime();
                dimmer.getState() ? jsonResult["state"] = "ON" : jsonResult["state"] = "OFF";

                String jsonReply;
                serializeJson(jsonResult, jsonReply);
                client.println(jsonReply);
              }

              if (root["method"] == "set_mode") {
                if (root.containsKey("mode")) if (root["mode"] == "NORMAL_MODE") dimmer.setMode(NORMAL_MODE); else dimmer.setMode(TOGGLE_MODE);
                StaticJsonDocument<200> jsonResult;

                jsonResult["id"] = root["id"];
                dimmer.getMode() ? jsonResult["result"] = "TOGGLE_MODE" : jsonResult["result"] = "NORMAL_MODE";

                String jsonReply;
                serializeJson(jsonResult, jsonReply);
                client.println(jsonReply);
              }

              if (root["method"] == "update") {
                if (root.containsKey("IP") & root.containsKey("url")) {
                  StaticJsonDocument<200> jsonResult;

                  jsonResult["id"] = root["id"];
                  jsonResult["result"] = "OK";

                  String jsonReply;
                  serializeJson(jsonResult, jsonReply);
                  client.println(jsonReply);

                  ESPhttpUpdate.update(root["IP"], 80, root["url"]);
                  ESP.restart();
                }
              }

            } json = "";  // Parse successfully
          }               // Json string
        }                 // Client connected

    client.stop();
    Serial.println("Client disconnected");
  }
  yield();
}
