Data Streamer for ESP8266 with nodemcu firmware for use with InitialState API

This repository borrows heavily from the example scripts at https://github.com/nodemcu/nodemcu-firmware/.

Uses a ds18b20 onewire sensor attached to GPIO2 on the ESP-01 module. 

ESP8266 ESP-01 pin connections
-------------------------------
VCC		3V3
GND		GND
RST		3V3
CH_PD		3V3
TX		RX serial
RX		TX serial
GPIO2		ds18b20


David Reeves 2015

************************

(1) Compile nodemcu firmware (https://github.com/nodemcu/nodemcu-firmware/) to include the cjson module. I used an online compiler (http://frightanic.com/nodemcu-custom-build/) and included modules as follows: node,file,gpio,wifi,net,pwm,tmr,uart,ow,cjson. 

(2) Connect your ESP8266 module appropriately to power and serial. I use a Sparkfun FTDI basic for this. Pull down GPIO0 to GND and power cycle the module to enable the bootloader. Reflash your ESP8266 with the nodemcu firmware. I used esptool (https://github.com/themadinventor/esptool) which requires a Python environment. Release GPIO0 and power cycle the module. 

(3) Edit the files in this repository to include your wifi credentials and Initial State API key and make up and include a bucket name and bucket key at the indicated places at the top of each lua file. 

(4) Upload the three lua files to the esp8266. I use luatool for this (https://github.com/4refr0nt/luatool) which also requires Python.

(5) use the command 'node.restart()' in serial terminal to reboot the esp8266. 

(6) wait for a wifi connection ('UP'), then use either 'dofile("bucket.lua")' if you haven't created the bucket yet, or 'dofile("stream.lua")' if you have already created it. If you get any errors, try rebooting the module to clear the heap and then run the appropriate script again. To have the 'stream.lua' file run automatically, remove the comment '--' from the dofile command in init.lua and reupload it. Serial connection is no longer required, but can be used to monitor the uploads if desired.