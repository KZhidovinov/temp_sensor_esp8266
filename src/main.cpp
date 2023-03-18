#include <Arduino.h>
#include <Esp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

#define ONE_WIRE_BUS 2

#define EEPROM_SIZE 8 // bytes
#define EEPROM_TEMP_ADDRESS 0

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
  delay(50);
}

void setup()
{
  Serial.begin(74880);
  Serial.flush();

  Serial.println("Wake up");

  dsSensor.begin();

  init_storage();
}

void loop()
{
  dsSensor.requestTemperatures();

  float prevTemp = get_saved_temp();
  float temp = dsSensor.getTempCByIndex(0);

  Serial.printf("Prev temp C: %5.3f, current temp C: %5.3f, delta: %5.3f\n", prevTemp, temp, temp - prevTemp);

  save_temp(temp);
  delay(1000);
}