#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Esp.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

#include "secrets.h"

#define DEBUG_TEMP_SENSOR 0
#if DEBUG_TEMP_SENSOR
#define DEEP_SLEEP_DELAY 15 * 1000000ULL // seconds
#define TEMP_DELTA 1e-6
#else
#define DEEP_SLEEP_DELAY 30 * 60 * 1000000ULL // seconds
#define TEMP_DELTA 0.2
#endif

#define WIFI_IS_OFF_AT_BOOT 1

#define LED_PIN 2
#define ONE_WIRE_BUS 2

#define EEPROM_SIZE 4096 // bytes
#define EEPROM_TEMP_ADDRESS 4000

#define SERIAL_SPEED 74880

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dsSensor(&oneWire);

/// @brief Init EEPROM storage
void init_storage()
{
  EEPROM.begin(EEPROM_SIZE);
  delay(50);
}

/// @brief Get saved temperature
/// @return Temperature C
float get_saved_temp()
{
  float temp = 0;
  return EEPROM.get(EEPROM_TEMP_ADDRESS, temp);
}

/// @brief Save temperature to EEPROM
/// @param temp Temperature C
void save_temp(float temp)
{
  EEPROM.put(EEPROM_TEMP_ADDRESS, temp);

  EEPROM.commit();
  delay(300);
}

void sleep()
{
  Serial.println(F("Go to sleep"));
  ESP.deepSleep(DEEP_SLEEP_DELAY);
}

void setup()
{
  Serial.begin(SERIAL_SPEED);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  bool ledOn = HIGH;

  init_storage();

  dsSensor.begin();
  delay(100);

  dsSensor.requestTemperatures();
  float prevTemp = get_saved_temp();
  float temp = dsSensor.getTempCByIndex(0);

  Serial.printf(PSTR("Prev temp C: %5.3f, current temp C: %5.3f, delta: %5.3f\n"), prevTemp, temp, temp - prevTemp);

  if (abs(temp - prevTemp) < TEMP_DELTA)
  {
    return;
  }

  station_config config;
  if (wifi_station_get_config(&config))
  {
    // Serial.printf("SSID: %s, PWD: %s", config.ssid, config.password);
  }

  WiFi.mode(WIFI_STA);

  if (config.ssid[0] == 0)
  {
    if (WiFi.beginSmartConfig())
    {
      while (!WiFi.smartConfigDone())
      {
        ledOn = !ledOn;
        digitalWrite(LED_PIN, ledOn);
        delay(50);
        Serial.print(F("."));
      }
    }
  }
  else
  {
    WiFi.begin();

    ledOn = LOW;
    digitalWrite(LED_PIN, ledOn);

    while (WiFi.status() != WL_CONNECTED)
    {
      ledOn = !ledOn;
      digitalWrite(LED_PIN, ledOn);
      delay(200);
      Serial.print(F("."));
    }

    ledOn = LOW;
    digitalWrite(LED_PIN, ledOn);
  }

  WiFiClientSecure *espClient = new WiFiClientSecure();

#ifdef MQTT_SERVER_FINGERPRINT
  espClient->setFingerprint(PSTR(MQTT_SERVER_FINGERPRINT));
#endif

#if defined(MQTT_CLIENT_CERT) && defined(MQTT_CLIENT_KEY)
  auto certificate = X509List(PSTR(MQTT_CLIENT_CERT));
  auto privateKey = PrivateKey(PSTR(MQTT_CLIENT_KEY));
  espClient->setClientRSACert(&certificate, &privateKey);
#endif

  PubSubClient *client = new PubSubClient(*espClient);

  char client_id[13];
  sprintf(client_id, PSTR("ESP_%08x"), ESP.getChipId());

  char message[10];
  sprintf(message, PSTR("%+4.1f"), temp);

  // Serial.println(F("Publish temp to test"));
  // Serial.println(message);

  char topic[100];
  sprintf(topic, PSTR(MQTT_TOPIC_PATTERN), client_id);

  client->setServer(MQTT_SERVER, MQTT_PORT);

#if defined(MQTT_USER) && defined(MQTT_PASSWORD)
  client->connect(client_id, MQTT_USER, MQTT_PASSWORD);
#else
  client->connect(client_id);
#endif

  client->publish(topic, message, true);

  client->disconnect();

  delete client;
  delete espClient;

  save_temp(temp);
}

void loop()
{
  sleep();
}
