#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
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

// DHT22 settings
#define DHT_PIN 3
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

bool isTransmitMode = true; // Start in receive mode
unsigned long lastSwitchTime = 0;
bool waitForTransmit = false; // Flag to wait before switching to transmit mode

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor

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
  // Initialize DHT22
  dht.begin();
}

void loop() {
  unsigned long currentTime = millis();

  if (isTransmitMode) {
    // Transmit mode: Read temperature and humidity and send data
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT22!");
      return;
    }

  // Convert to integers (multiply by 100)
  int tempInt = (int)(temperature * 100);
  int humidityInt = (int)(humidity * 100);

  // Print readings to serial monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.print("Temp:");
  LoRa.print(tempInt);
  LoRa.print(",Humidity:");
  LoRa.print(humidityInt);
  LoRa.endPacket();

    
    display.clear();
    display.println("Transmitting..");
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");
    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");
    
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
      
      // Extract ADC and Voltage using String methods
     int adcIndex = incomingData.indexOf("ADC:") + 4; // Start of ADC value
     int commaIndex = incomingData.indexOf(",");      // Separator between ADC and Voltage
     int voltageIndex = incomingData.indexOf("Voltage:") + 8; // Start of Voltage value

    if (adcIndex != -1 && commaIndex != -1 && voltageIndex != -1) {
     int adcValue = incomingData.substring(adcIndex, commaIndex).toInt();
     float voltage = incomingData.substring(voltageIndex).toFloat();

      Serial.print("Parsed ADC: ");
      Serial.print(adcValue);
      Serial.print(", Voltage: ");
      Serial.print(voltage);
      Serial.print(" v, RSSI: ");
      Serial.print(rssi);
      Serial.println(" dBm");
      
      
      display.clear();
      display.println("Receiving..");
      display.print("ADC: ");
      display.println(adcValue);
      display.print("Voltage: ");
      display.print(voltage);
      display.println(" v");
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