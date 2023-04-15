# temp_sensor_esp8266
A temperatire sensor built on DS18B20 sensor and ESP8266 MCU (ESP-01)

## Decription

This firmware requests temperature then compares it with value saved in EEPROM. 

If delta more than TEMP_DELTA then current temperature will be sent to MQTT topic and will be saved to EEPROM.

After that device is going to deep sleep for DEEP_SLEEP_DELAY seconds.

## Build

Add `include/secrets.h`

```cpp
#pragma once

#define MQTT_SERVER "you-mqtt-server"
#define MQTT_PORT 1883
#define MQTT_USER "mqtt-user"
#define MQTT_PASSWORD "mqtt-password"

#define SERVER_FINGERPRINT "mqtt-server's-certificate-fingerprint"
```
