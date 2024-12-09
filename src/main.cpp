#include <Arduino.h>
#include <FastLED.h>
#include <DHT20.h>
#include <PulseSensorPlayground.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#define LED_PIN 13
#define NUM_LEDS 166
#define BULB_PIN 17 // LED to blink on motion
#define PULSE_INPUT 32
#define THRESHOLD 525
#define MOTION_SENSOR_PIN 15 // IR motion sensor input pin
#define BUTTON 2

// Cloud storage configuration
char ssid[50];                              // your network SSID (name)
char pass[50];                              // your network password
const char kHostname[] = "204.236.147.158"; // Server hostname
char kPath[200];                            // Path for data transmission
const int kNetworkTimeout = 30 * 1000;
const int kNetworkDelay = 1000;

CRGB leds[NUM_LEDS];

// PulseSensor setup
// PulseSensorPlayground pulseSensor;

// DHT20 setup
DHT20 DHT;

// Motion sensor state
int pirState = LOW; // Start with no motion detected
int state = 0;      // 0 for temp, 1 for heart rate

void nvs_access()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  // Open
  Serial.printf("\nOpening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
    switch (err)
    {
    case ESP_OK:
      Serial.printf("WiFi credentials retrieved successfully\n");
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      Serial.printf("The WiFi credentials are not initialized yet!\n");
      break;
    default:
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }
  // Close
  nvs_close(my_handle);
}

void sendDataToCloud(float temperature, float humidity, int heartRate)
{
  int err = 0;
  WiFiClient c;
  HttpClient http(c);

  // Prepare path with all sensor data
  snprintf(kPath, sizeof(kPath),
           "/?temperature=%.2f&humidity=%.2f&heartrate=%d",
           temperature, humidity, heartRate);

  err = http.get(kHostname, 5000, kPath, NULL);
  if (err == 0)
  {
    Serial.println("Cloud data transmission started");
    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Server response code: ");
      Serial.println(err);

      // Optional: process server response if needed
      http.skipResponseHeaders();
    }
    else
    {
      Serial.print("Failed to get response: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Cloud connection failed: ");
    Serial.println(err);
  }
  http.stop();
}

void setup()
{
  analogReadResolution(10);

  // FastLED setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  // Serial communication
  Serial.begin(9600);
  delay(1000);

  // Retrieve WiFi credentials from NVS
  nvs_access();

  // WiFi connection
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Bulb pin and PulseSensor setup
  pinMode(BULB_PIN, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);

  // DHT20 setup
  Wire.begin();
  DHT.begin();

  // Turn off the LED strip initially
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void setLEDColor(float temperature)
{
  if (temperature > 74)
  { // Hot temperatures (orange to red)
    for (int i = 0; i <= 166; i++)
    { // Orange goes one round
      leds[i] = CRGB(255, 165, 0);
      FastLED.show();
      delay(5);
    }
    for (int i = 0; i <= 166; i++)
    { // Red goes one round
      leds[i] = CRGB(160, 0, 0);
      FastLED.show();
      delay(5);
    }
  }
  else if (temperature > 72)
  { // Warm temperatures (green to yellow)
    for (int i = 0; i <= 166; i++)
    { // Green goes one round
      leds[i] = CRGB(0, 255, 0);
      FastLED.show();
      delay(5);
    }
    for (int i = 0; i <= 166; i++)
    { // Yellow goes one round
      leds[i] = CRGB(255, 255, 0);
      FastLED.show();
      delay(5);
    }
  }
  else
  { // Cold temperatures (blue to purple)
    for (int i = 0; i <= 166; i++)
    { // Blue goes one round
      leds[i] = CRGB(0, 0, 255);
      FastLED.show();
      delay(5);
    }
    for (int i = 0; i <= 166; i++)
    { // Purple goes one round
      leds[i] = CRGB(160, 0, 255);
      FastLED.show();
      delay(5);
    }
  }
}

void loop()
{
  int buttonState = digitalRead(BUTTON);
  int motionDetected = digitalRead(MOTION_SENSOR_PIN);

  if (motionDetected == HIGH)
  {
    if (buttonState == HIGH && state == 0)
    {
      state = 1;
      Serial.println("State switched to heart rate");
    }

    else if (buttonState == HIGH && state == 1)
    {
      state = 0;
      Serial.println("State switched to temperature");
    }

    if (pirState == LOW)
    {
      Serial.println("Motion detected!");
      pirState = HIGH;
    }

    if (state == 0)
    {
      // Read temperature and humidity
      DHT.read();
      float temperatureC = DHT.getTemperature();
      float temperature = (temperatureC * 1.8) + 32; // Convert to Fahrenheit
      float humidity = DHT.getHumidity();

      // Set LED color based on temperature
      setLEDColor(temperature);

      // Send temperature and humidity to cloud
      sendDataToCloud(temperature, humidity, 0);
    }
    else
    {
      // Heart rate mode
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  }
  else
  {
    if (pirState == HIGH)
    {
      Serial.println("Motion ended!");
      pirState = LOW;
      // Turn off the LED strip when no motion is detected
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  }

  delay(100);
}