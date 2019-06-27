# ESP8266 DIY AC Dimmer 220V
ESP8266 DIY Smart Home AC Dimmer 220V
DIY Smart Home based on Espressif Systems (ESP32, ESP8266) Domoticz plugin

## Connection diagram for ESP Smart Home DIY AC dimmer

![connection diagram](https://github.com/Whilser/ESP-DIY-Samrt-Home/raw/master/images/ESPDIYSmartHome.png)

SSH commands:

    {"id":1, "method":"set_power", "power":"50", "state":"ON"}
    {"id":1, "method":"set_power", "power":"50", "state":"OFF"}
    {"id":1, "method":"set_state", "state":"OFF"}
    {"id":1, "method":"set_state", "state":"ON"}
    {"id":1, "method":"set_config", "SSID":"Wi-Fi SSID", "PASSWD": "PASSWORD"}
    {"id":1, "method":"set_mode", "mode":"TOGGLE_MODE"}
    {"id":1, "method":"get_temperature"}
    {"id":1, "method":"get_state"}
    {"id":1, "method":"update", "IP":"192.168.4.1", "url":"/update/firmware.bin"}
