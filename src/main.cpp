// #include <Arduino.h>



// const int LED_PIN = 17;

// void setup(){
//   pinMode(LED_PIN, OUTPUT);
// }

// void loop(){
//   digitalWrite(LED_PIN, 1);

// }

#include <Arduino.h>
#include <FastLED.h>
#include <VoiceRecognitionV3.h>
#include <SoftwareSerial.h>
#define LED_PIN     13
#define NUM_LEDS    166
#define BULB_PIN 17

SoftwareSerial voiceRecognition(22, 21); // RX on pin 22, TX on pin 21

CRGB leds[NUM_LEDS];
void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(9600);
  voiceRecognition.begin(9600);
  pinMode(BULB_PIN, OUTPUT);
}

void handleCommand(String command)
{
  if (command == "hello")
  {
    Serial.println("Hello, Voice recognition module!");
  }
  else if (command == "blink")
  {
    digitalWrite(LED_PIN, HIGH); // LED_PIN is the pin number of the LED you want to blink
    delay(1000);                 // delay for 1 second (1000 milliseconds)
    digitalWrite(LED_PIN, LOW);  // turn the LED off again
  }
  else
  {
    Serial.println("Unknown command");
  }
}

void loop() {
  digitalWrite(BULB_PIN, 1);

  if (voiceRecognition.available())
  {
    String command = voiceRecognition.readString();
    Serial.println("Command received:" + command);
    handleCommand(command);
  }

  for (int i = 0; i <= 166; i++) {         //Red goes one round
    leds[i] = CRGB ( 160, 32, 255);
    FastLED.show();
    delay(5);
  }
  for (int i = 0; i <= 166; i++) {         //Green goes one round 
    leds[i] = CRGB ( 80, 208, 255);
    FastLED.show();
    delay(5);
  }
  for (int i = 0; i <= 166; i++) {         //Blue goes one round 
    leds[i] = CRGB ( 255, 0, 0);
    FastLED.show();
    delay(5);
  }
}