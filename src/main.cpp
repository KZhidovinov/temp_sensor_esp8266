#include <Arduino.h>
#include <Esp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature dsSensor(&oneWire);

void setup() {
  Serial.begin(74880);
  Serial.flush();

  Serial.println("Wake up");

  Serial.println("Init temp sensor");
  dsSensor.begin();
  Serial.print("...done.\n");
}

void loop() {
  dsSensor.requestTemperatures();

  Serial.printf("Temp C: %5.3f\n", dsSensor.getTempCByIndex(0));

  delay(1000);
}