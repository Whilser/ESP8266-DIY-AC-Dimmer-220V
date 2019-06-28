# ESP8266 DIY Smart Home Zero-Cross AC Dimmer 220V

DIY Smart Home based on Espressif Systems (ESP32, ESP8266) Family device

## Connection diagram for ESP Smart Home DIY AC dimmer

**Hardware:**

- [x] Espressif Systems ESP8266 (NodeMCU)
- [x] RobotDyn Thyristor AC

![connection diagram](https://github.com/Whilser/ESP-DIY-Samrt-Home/raw/master/images/ESPDIYSmartHome.png)

![Pinout](https://robotdyn.com/pub/media/0G-00005677==Mod-Dimmer-5A-1L/DOCS/PINOUT==0G-00005677==Mod-Dimmer-5A-1L.jpg)

Control through SSH commands:

    {"id":1, "method":"set_power", "power":"50", "state":"ON"}
    {"id":1, "method":"set_power", "power":"50", "state":"OFF"}
    {"id":1, "method":"set_state", "state":"OFF"}
    {"id":1, "method":"set_state", "state":"ON"}
    {"id":1, "method":"set_config", "SSID":"Wi-Fi SSID", "PASSWD": "PASSWORD"}
    {"id":1, "method":"set_mode", "mode":"TOGGLE_MODE"}
    {"id":1, "method":"get_temperature"}
    {"id":1, "method":"get_state"}
    {"id":1, "method":"update", "IP":"192.168.4.1", "url":"/update/firmware.bin"}
