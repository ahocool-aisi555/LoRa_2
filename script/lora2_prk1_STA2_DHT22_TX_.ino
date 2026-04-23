//by : Nyoman Yudi Kurniawan
//for : TRSE - Undiksha - 2025
//www.aisi555.com

#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// LoRa pins
#define LORA_CS_PIN 10
#define LORA_RST_PIN 9
#define LORA_DIO0_PIN 2

// DHT22 settings
#define DHT_PIN 3
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// OLED settings
#define I2C_ADDRESS 0x3C
#define RST_PIN -1

SSD1306AsciiWire oled;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial monitor to open


  Wire.begin();
  Wire.setClock(400000L);
  
  oled.begin(&Adafruit128x64, I2C_ADDRESS);

  oled.setFont(Arial14);
  oled.clear();
  oled.println("Initializing DHT TX...");
  
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

  // Initialize DHT22
  dht.begin();
}

void loop() {
  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if readings are valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT22!");
    delay(2000); // Wait before retrying
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

    // Update OLED with transmited  data
   oled.clear();
   oled.println("Transmiting..");
   oled.print("Temp: ");
   oled.print(temperature);
   oled.println(" C");
   oled.print("Humidity: ");
   oled.print(humidity);
   oled.println(" %");
  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.print("Temp:");
  LoRa.print(tempInt);
  LoRa.print(",Humidity:");
  LoRa.print(humidityInt);
  LoRa.endPacket();

  delay(5000); // Wait 5 seconds before sending again
}
