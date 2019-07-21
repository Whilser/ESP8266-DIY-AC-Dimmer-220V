# ESP8266 DIY Smart Home Zero-Cross AC Dimmer 220V

DIY Smart Home based on Espressif Systems (ESP32, ESP8266) Family device

**Hardware:**

- [x] Espressif Systems ESP8266 (NodeMCU)
- [x] RobotDyn Thyristor AC

**Discover python script:**

```
import json
import socket

print('Starting discover ESP8266 DIY Smart Home devices...')
helobytes = 'discover'.encode()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
s.settimeout(5)
s.sendto(helobytes, ('<broadcast>', 1000))

data, addr = s.recvfrom(1024)

reply = json.loads(data.decode())
deviceID = reply["deviceID"]
IP = reply["IP"]
hardware = reply["hardware"]

print('Discovered. Device ID: {0}, IP: {1}, Model: {2}'.format(deviceID, IP, hardware))

```

### Connection diagram for ESP Smart Home DIY AC dimmer

![connection diagram](https://github.com/Whilser/ESP-DIY-Samrt-Home/raw/master/images/ESPDIYSmartHome.png)

![Pinout](https://robotdyn.com/pub/media/0G-00005677==Mod-Dimmer-5A-1L/DOCS/PINOUT==0G-00005677==Mod-Dimmer-5A-1L.jpg)


**Home Assistant integration**

```
light:
  - platform: mqtt
    name: "DIY AC Dimmer 220V"
    state_topic: "DIYSmartHomeACDimmer/light/status"
    command_topic: "DIYSmartHomeACDimmer/light/switch"
    brightness_state_topic: "DIYSmartHomeACDimmer/brightness/status"
    brightness_command_topic: "DIYSmartHomeACDimmer/brightness/set"
    payload_on: "ON"
    payload_off: "OFF"

sensor:
  - platform: mqtt
    name: "Temperature"
    state_topic: "DIYSmartHomeACDimmer/sensor/temperature"
    unit_of_measurement: "ºC"
```

![HomeAssistant](https://github.com/Whilser/ESP8266-DIY-AC-Dimmer-220V/raw/master/images/DIY_Dimmer.png)


**Control through SSH commands:**

    {"id":1, "method":"set_power", "power":"50", "state":"ON"}
    {"id":1, "method":"set_power", "power":"50", "state":"OFF"}
    {"id":1, "method":"set_state", "state":"OFF"}
    {"id":1, "method":"set_state", "state":"ON"}
    {"id":1, "method":"set_config", "SSID":"Wi-Fi SSID", "PASSWD": "PASSWORD"}
    {"id":1, "method":"set_mqtt", "host": "MQTT server", "USER": "MQTT_USER", "PASSWD": "MQTT_PASSWD", "ROOT": "DIYSmartHomeACDimmer"}' | nc -w1 <IP ESP8266> 2000
    {"id":1, "method":"set_mode", "mode":"TOGGLE_MODE"}
    {"id":1, "method":"get_temperature"}
    {"id":1, "method":"get_state"}
    {"id":1, "method":"update", "IP":"<Update Server IP>", "url":"/update/firmware.bin"}
    
 ### Connecting the device to the Wi-Fi network
 
In case of unsuccessful connection to the Wi-Fi network, the device creates an access point with an ip address 192.168.4.1. To send Wi-Fi network settings (SSID, PASSWORD) to the device, connect to the AP and send a command in SSH terminal:

    echo '{"id":1, "method":"set_config", "SSID":"Wi-Fi SSID", "PASSWD": "PASSWORD"}' | nc -w1 <deviceIP> 2000
 
 ### MQTT Configuration

Just send a command in SSH terminal:

    echo '{"id":1, "method":"set_mqtt", "host":"MQTT server", "USER": "MQTT_USER", "PASSWD": "MQTT_PASSWD", "ROOT": "DIYSmartHomeACDimmer"}' | nc -w1 <IP ESP8266> 2000

**MQTT server** - Address of MQTT server, **MQTT_USER** - MQTT username, **MQTT_PASSWD** - password to access to MQTT server, **ROOT** - root topic of device, example: DIYSmartHomeACDimmer. 

**Examples:**
 
* Get state

        echo '{"id":1, "method":"get_state"}' | nc -w1 <deviceIP> 2000

* Switch ON

        echo '{"id":1, "method":"set_state", "state":"ON"}' | nc -w1 <deviceIP> 2000

* Switch OFF

        echo '{"id":1, "method":"set_state", "state":"OFF"}' | nc -w1 <deviceIP> 2000

* Get temperature

        echo '{"id":1, "method":"get_temperature"}' | nc -w1 <deviceIP> 2000

* Set Power of 50%

        echo '{"id":1, "method":"set_power", "power":"50", "state":"ON"}' | nc -w1 <deviceIP> 2000
        
In case of temperature exceeding of 40 degrees celsius, the device will automatically turn itself OFF and stop receiving commands until the temperature drops.

