
import json
import socket

print('Starting discover ESP8266 DYI Smart Home devices...')
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
