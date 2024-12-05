#include <Arduino.h>
#include <FastLED.h>
#include <VoiceRecognitionV3.h>
#include <SoftwareSerial.h>
#include <PulseSensorPlayground.h>

#define LED_PIN 13
#define NUM_LEDS 166
#define BULB_PIN 17 // LED to blink on pulse or motion
#define PULSE_INPUT 32
#define THRESHOLD 525
#define MOTION_SENSOR_PIN 15 // IR motion sensor input pin

SoftwareSerial voiceRecognition(22, 21); // RX on pin 22, TX on pin 21
CRGB leds[NUM_LEDS];

// PulseSensor setup
PulseSensorPlayground pulseSensor;

// Motion sensor state
int pirState = LOW; // Start with no motion detected

void setup()
{
  // FastLED setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  // Serial communication
  Serial.begin(9600); // Set baud rate to 9600
  voiceRecognition.begin(9600);

  // Bulb pin and PulseSensor setup
  pinMode(BULB_PIN, OUTPUT);         // Set BULB_PIN as output
  pinMode(MOTION_SENSOR_PIN, INPUT); // Set motion sensor as input
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(-1); // Disable library's blink-on-pulse feature
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(SERIAL_PLOTTER);
  pulseSensor.setThreshold(THRESHOLD);

  // Disable hardware timing
  pulseSensor.UsingHardwareTimer = false;

  // Initialize PulseSensor
  if (!pulseSensor.begin())
  {
    while (true)
    {
      // Indicate initialization failure by blinking BULB_PIN
      digitalWrite(BULB_PIN, LOW);
      delay(100);
      digitalWrite(BULB_PIN, HIGH);
      delay(100);
    }
  }

  // Turn off the LED strip initially
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void handleCommand(String command)
{
  if (command == "hello")
  {
    Serial.println("Hello, Voice recognition module!");
  }
  else if (command == "blink")
  {
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
  }
  else if (command == "brightness low")
  {
    FastLED.setBrightness(50);
    Serial.println("Brightness set to low.");
  }
  else if (command == "brightness high")
  {
    FastLED.setBrightness(200);
    Serial.println("Brightness set to high.");
  }
  else
  {
    Serial.println("Unknown command");
  }
}

void loop()
{
  // // Handle voice commands
  // if (voiceRecognition.available())
  // {
  //   String command = voiceRecognition.readString();
  //   Serial.println("Command received: " + command);
  //   handleCommand(command);
  // }

  // Heartbeat detection
  if (pulseSensor.sawStartOfBeat())
  {
    Serial.println("Heartbeat detected!");
    // pulseSensor.outputBeat();

    // Blink LED on BULB_PIN for heartbeat
    digitalWrite(BULB_PIN, HIGH);
    delay(50);
    digitalWrite(BULB_PIN, LOW);
  }

  // Motion detection
  int motionDetected = digitalRead(MOTION_SENSOR_PIN); // Read motion sensor
  if (motionDetected == HIGH)
  {
    fill_solid(leds, NUM_LEDS, CRGB::White); // Turn on LED strip (set to white)
    FastLED.show();

    if (pirState == LOW)
    {
      Serial.println("Motion detected!");
      pirState = HIGH;
    }
  }
  else
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black); // Turn off LED strip
    FastLED.show();

    if (pirState == HIGH)
    {
      Serial.println("Motion ended!");
      pirState = LOW;
    }
  }

  // Heartbeat sensor periodic update
  if (pulseSensor.UsingHardwareTimer)
  {

    /*
       Wait a bit.
       We don't output every sample, because our baud rate
       won't support that much I/O.
    */
    delay(20);
    // write the latest sample to Serial.
    pulseSensor.outputSample();
  }
  else
  {
    /*
        When using a software timer, we have to check to see if it is time
        to acquire another sample. A call to sawNewSample will do that.
    */
    if (pulseSensor.sawNewSample())
    {
      /*
          Every so often, send the latest Sample.
          We don't print every sample, because our baud rate
          won't support that much I/O.
      */
      if (--pulseSensor.samplesUntilReport == (byte)0)
      {
        pulseSensor.samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
        pulseSensor.outputSample();
      }
    }
  }
}
