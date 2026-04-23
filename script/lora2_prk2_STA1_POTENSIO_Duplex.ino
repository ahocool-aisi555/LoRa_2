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

bool isTransmitMode = true; // Start in transmit mode
unsigned long lastSwitchTime = 0;
bool waitForTransmit = false; // Flag to wait before switching to transmit mode

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
  unsigned long currentTime = millis();

  if (isTransmitMode) {
    // Transmit mode: Read potentiometer and send data
    int adcValue = analogRead(POT_PIN);
    float voltage = (adcValue / 4095.0) * REFERENCE_VOLTAGE;

    Serial.print("Sending ADC: ");
    Serial.print(adcValue);
    Serial.print(", Voltage: ");
    Serial.print(voltage);
    Serial.println(" v");

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
    
    // Switch to receive mode immediately after transmitting
    isTransmitMode = false;
  } else {
    // Receive mode: Listen for incoming data
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String incomingData = "";
      while (LoRa.available()) {
        incomingData += (char)LoRa.read();
      }
      // Get RSSI
      int rssi = LoRa.packetRssi();
      
      Serial.print("Incoming Data: ");
      Serial.println(incomingData);

   
     // Parse temperature and humidity as integers
    int tempInt = 0, humidityInt = 0;
    int parsed = sscanf(incomingData.c_str(), "Temp:%d,Humidity:%d", &tempInt, &humidityInt);

    if (parsed == 2) { // Ensure both values were parsed successfully
      // Convert integers back to floats
      float temperature = tempInt / 100.0;
      float humidity = humidityInt / 100.0;

      Serial.print("Parsed Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.print(" %, RSSI: ");
      Serial.print(rssi);
      Serial.println(" dBm");

            // Update OLED with received data
      display.clear();
      display.println("Receiving...");
      display.print("Temp: ");
      display.print(temperature);
      display.println(" C");
      display.print("Humidity: ");
      display.print(humidity);
      display.println(" %");
      display.print("RSSI: ");
      display.print(rssi);
      display.println(" dBm");

      
    } else {
      Serial.println("Failed to parse data!");
    }



      // Set flag to wait for 5 seconds before switching to transmit mode
      waitForTransmit = true;
      lastSwitchTime = currentTime;
    }

    // Check if 5 seconds have passed since receiving data
    if (waitForTransmit && (currentTime - lastSwitchTime >= 5000)) {
      isTransmitMode = true; // Switch to transmit mode
      waitForTransmit = false; // Reset flag
    }
  }
}