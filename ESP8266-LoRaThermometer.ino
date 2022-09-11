#include <SPI.h>
#include <LoRa.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include "defines.h"

int counter = 0;

OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensors(&oneWire);

void setup() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println("");
    Serial.println("Setup...");
  }

  setupLoRa();

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Setup Completed");
  }
}

void setupLoRa() {
  LoRa.setPins(SS_PIN, RESET_PIN, IRQ_PIN);
  
  if (!LoRa.begin(LORA_FREQUENZY)) {
    if(DEBUG_PRINT_SERIAL) {
      Serial.println("Starting LoRa failed!");
      Serial.println("Rebooting ESP");
    }
    ESP.restart();
  }

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("\nLoRa Connected");
    Serial.print("\tRSSI:\t");
    Serial.println(LoRa.rssi());
  }
}

void readSensors() {
  sensors.requestTemperatures(); 
  float temperature_Celsius = sensors.getTempCByIndex(0);

  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Temperature: ");
    Serial.print(temperature_Celsius);
    Serial.println(" ºC");
  }

  StaticJsonDocument<32> doc;
  doc["temperature"] = temperature_Celsius;
  doc["seq"] = counter;
  String output = "";
  serializeJson(doc, output);

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Sending LoRa:");
    Serial.print(output);
  }

  LoRa.beginPacket();
  LoRa.print(output);
  LoRa.endPacket();
}

void loop() {
  readSensors();
  counter++;

  delay(LOOP_DELAY_SECONDS * 1000);
}
