//by : Nyoman Yudi Kurniawan
//for : TRSE - Undiksha - 2025
//www.aisi555.com

#include <SPI.h>
#include <LoRa.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"



// LoRa pins
#define LORA_CS_PIN 10
#define LORA_RST_PIN 9
#define LORA_DIO0_PIN 2

// OLED settings
#define I2C_ADDRESS 0x3C
#define RST_PIN -1

SSD1306AsciiWire oled;


void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor to open

  // Initialize OLED

  Wire.begin();
  Wire.setClock(400000L);
  
  oled.begin(&Adafruit128x64, I2C_ADDRESS);

  
  oled.setFont(Arial14);
  oled.clear();
  oled.println("Initializing Gateway...");


  // Initialize LoRa
  SPI.begin();
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(433E6)) { // Set frequency to 433 MHz
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  Serial.println("LoRa initialized successfully.");

  oled.println("Lora OK !");
  delay(2000);
}

void loop() {
  // Check for incoming LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incomingData = "";
    while (LoRa.available()) {
      incomingData += (char)LoRa.read();
    }

    // Get RSSI value
    int rssi = LoRa.packetRssi();
    
    // Debugging: Print raw received data
    //Serial.print("Raw received data: ");
    //Serial.println(incomingData);

      // Extract ADC and Voltage using String methods
     int adcIndex = incomingData.indexOf("ADC:") + 4; // Start of ADC value
     int commaIndex = incomingData.indexOf(",");      // Separator between ADC and Voltage
     int voltageIndex = incomingData.indexOf("Voltage:") + 8; // Start of Voltage value

    if (adcIndex != -1 && commaIndex != -1 && voltageIndex != -1) {
     int adcValue = incomingData.substring(adcIndex, commaIndex).toInt();
     float voltage = incomingData.substring(voltageIndex).toFloat();
      
      Serial.print("{ \"ADC\": ");
      Serial.print(adcValue);
      Serial.print(", \"Voltage\": ");
      Serial.print(voltage);
      Serial.print(", \"Rssi\": ");
      Serial.print(rssi);
      Serial.println("}");
    
          // Update OLED with received data
      oled.clear();
      oled.println("Received Data..");
      oled.print("ADC: ");
      oled.println(adcValue);
      oled.print("Voltage: ");
      oled.print(voltage);
      oled.println(" v");
      oled.print("Rssi: ");
      oled.print(rssi);
      oled.println(" dBm");

    
    } else {
      Serial.println("Failed to parse data!");
    }
  }
}
