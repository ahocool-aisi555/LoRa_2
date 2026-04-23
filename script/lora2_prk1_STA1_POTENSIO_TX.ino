//by : Nyoman Yudi Kurniawan
//for : TRSE - Undiksha - 2025
//www.aisi555.com

#include <SPI.h>
#include <LoRa.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// OLED settings
#define I2C_ADDRESS 0x3C
#define RST_PIN -1

SSD1306AsciiWire display;

// LoRa pins
#define LORA_CS_PIN 10
#define LORA_RST_PIN 9
#define LORA_DIO0_PIN 2

// Potentiometer pin
#define POT_PIN A0
#define REFERENCE_VOLTAGE 3.3 


void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor

    // Configure ADC reference voltage
  analogReference(EXTERNAL); // Use external reference voltage (3.3V)

  // Set ADC resolution to 12 bits
  analogReadResolution(12);


  Wire.begin();
  Wire.setClock(400000L);
  
  display.begin(&Adafruit128x64, I2C_ADDRESS);
  
  
  display.setFont(Verdana12);
  display.clear();
  display.println("Initializing...");


  // Initialize LoRa
  SPI.begin();
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  Serial.println("LoRa initialized successfully.");

  display.println("Lora OK !");
}

void loop() {

 
    int adcValue = analogRead(POT_PIN);
    float voltage = (adcValue / 4095.0) * REFERENCE_VOLTAGE;

    Serial.print("Sending ADC: ");
    Serial.print(adcValue);
    Serial.print(", Voltage: ");
    Serial.println(voltage);

    LoRa.beginPacket();
    LoRa.print("ADC:");
    LoRa.print(adcValue);
    LoRa.print(",Voltage:");
    LoRa.print(voltage);
    LoRa.endPacket();

    // Update OLED with transmitted data
    display.clear();
    display.println("Transmitting..");
    display.print("ADC: ");
    display.println(adcValue);
    display.print("Voltage: ");
    display.print(voltage);
    display.println(" v");
    delay(5000);
}
