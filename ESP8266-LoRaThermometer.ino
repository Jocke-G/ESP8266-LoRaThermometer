#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <LoRa.h>
#include <OneWire.h>
#include <SPI.h>

#include "defines.h"
#define RTCMEMORYSTART 65
extern "C" {
#include "user_interface.h"
}

int seq = 0;

OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensors(&oneWire);

typedef struct {
  int seq;
} rtcStore;

rtcStore rtcMem;

void setup() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.begin(SERIAL_BAUDRATE);
    while(!Serial) { }
    Serial.println("\n=== setup() ===");
    Serial.print("Reset reason: ");
    Serial.println(ESP.getResetReason());
  }

  rst_info *resetInfo;
  resetInfo = ESP.getResetInfoPtr();
  if(resetInfo->reason == REASON_DEEP_SLEEP_AWAKE) {
    readFromRTCMemory();
  }

  setupLoRa();

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Setup Completed");
  }
}

void setupLoRa() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Starting LoRa...");
  }

  LoRa.setPins(SS_PIN, RESET_PIN, IRQ_PIN);

  if (!LoRa.begin(LORA_FREQUENZY)) {
    if(DEBUG_PRINT_SERIAL) {
      Serial.println("Starting LoRa failed!");
      Serial.println("Rebooting ESP");
    }
    ESP.restart();
  }

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("LoRa Connected");
    Serial.print("\tRSSI:\t");
    Serial.println(LoRa.rssi());
  }
}

void readFromRTCMemory() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Reading RTC");
  }
  system_rtc_mem_read(RTCMEMORYSTART, &rtcMem, sizeof(rtcMem));
  seq = rtcMem.seq;

  if(DEBUG_PRINT_SERIAL) {
    Serial.print("seq = ");
    Serial.println(rtcMem.seq);
  }
  yield();
}

void writeToRTCMemory() {
  rtcMem.seq = seq;
  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Writing RTC");
    Serial.print("seq = ");
    Serial.println(rtcMem.seq);
  }

  system_rtc_mem_write(RTCMEMORYSTART, &rtcMem, 4);
  yield();
}

void loop() {
  if(DEBUG_PRINT_SERIAL) {
    Serial.println("=== loop() ===");
  }

  readSensors();
  endLoop();
}

void readSensors() {
  sensors.requestTemperatures(); 
  float temperature_Celsius = sensors.getTempCByIndex(0);

  if(DEBUG_PRINT_SERIAL) {
    Serial.print("Temperature: ");
    Serial.print(temperature_Celsius);
    Serial.println(" ºC");
  }

  StaticJsonDocument<50> doc;
  doc["client"] = CLIENT_ID;
  doc["seq"] = seq;
  doc["temperature"] = temperature_Celsius;
  String output = "";
  serializeJson(doc, output);

  if(DEBUG_PRINT_SERIAL) {
    Serial.println("Sending LoRa:");
    Serial.println(output);
  }

  LoRa.beginPacket();
  LoRa.print(output);
  LoRa.endPacket();
}

void endLoop() {
    seq++;

  if(USE_DEEPSLEEP) {
    writeToRTCMemory();
  }

  if(USE_DEEPSLEEP) {
    if(DEBUG_PRINT_SERIAL) {
      Serial.print("Going to deep sleep for ");
      Serial.print(LOOP_DELAY_SECONDS);
      Serial.println(" seconde");
    }
    ESP.deepSleep(LOOP_DELAY_SECONDS * 1000000); 
  } else {
    if(DEBUG_PRINT_SERIAL) {
      Serial.print("Delay for ");
      Serial.print(LOOP_DELAY_SECONDS);
      Serial.println(" seconde");
    }
    delay(LOOP_DELAY_SECONDS * 1000);
  }
}
