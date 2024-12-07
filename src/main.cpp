#include <Arduino.h>
#include <FastLED.h>
#include <DHT20.h>
#include <PulseSensorPlayground.h>

#define LED_PIN 13
#define NUM_LEDS 166
#define BULB_PIN 17 // LED to blink on motion
#define PULSE_INPUT 32
#define THRESHOLD 525
#define MOTION_SENSOR_PIN 15 // IR motion sensor input pin
#define BUTTON 2

CRGB leds[NUM_LEDS];

// PulseSensor setup
PulseSensorPlayground pulseSensor;

// DHT20 setup
DHT20 DHT;

// Motion sensor state
int pirState = LOW; // Start with no motion detected

int state = 0; // 0 for temp, 1 for heart rate



void setup()
{

  analogReadResolution(10);
  // FastLED setup
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);

  // Serial communication
  Serial.begin(9600);

  // Bulb pin and PulseSensor setup
  pinMode(BULB_PIN, OUTPUT);         // Set BULB_PIN as output
  pinMode(MOTION_SENSOR_PIN, INPUT); // Set motion sensor as input
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(SERIAL_PLOTTER);
  pulseSensor.setThreshold(THRESHOLD);

  // Disable hardware timing
  pulseSensor.UsingHardwareTimer = false;

  pulseSensor.begin();
  // Initialize PulseSensor
  // if (!pulseSensor.begin())
  // {
  //   while (true)
  //   {
  //     // Indicate initialization failure by blinking BULB_PIN
  //     digitalWrite(BULB_PIN, LOW);
  //     delay(100);
  //     digitalWrite(BULB_PIN, HIGH);
  //     delay(100);
  //   }
  // }

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
  {
    fill_solid(leds, NUM_LEDS, CRGB::Red); // Set LEDs to red
  }
  else if (temperature > 72)
  {
    fill_solid(leds, NUM_LEDS, CRGB::Orange); // Set LEDs to orange
  }
  else
  {
    fill_solid(leds, NUM_LEDS, CRGB::Blue); // Set LEDs to blue
  }
  FastLED.show();
}

void loop()
{

  int buttonState = digitalRead(BUTTON);

  bool buttonPressed;

  
  
  // Serial.println("Button_press");

   

      // Motion detection
  int motionDetected = digitalRead(MOTION_SENSOR_PIN); // Read motion sensor
  if (motionDetected == HIGH)
  {
    if (buttonState == HIGH && state == 0){
      state = 1;
      Serial.println("State switched to heart rate");
    }
    
    else if (buttonState == HIGH && state == 1){
      state = 0;
      Serial.println("State switched to temperature");
    }

   // Serial.println("HIGH");
    if (pirState == LOW)
    {
      Serial.println("Motion detected!");
      pirState = HIGH;
    }

    if (state == 0){

      // Read temperature and humidity
      DHT.read();
      float temperatureC = DHT.getTemperature();
      float temperature = (temperatureC * 1.8) + 32; // Convert to Fahrenheit
      float humidity = DHT.getHumidity();

      // Serial.print("Temperature: ");
      // Serial.print(temperature);
      // Serial.println(" °F");

      // Serial.print("Humidity: ");
      // Serial.print(humidity);
      // Serial.println(" %");

      // Set LED color based on temperature
      setLEDColor(temperature);

    } else{
      // heart rate stuff
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      if(pulseSensor.sawStartOfBeat()){
        pulseSensor.blinkOnPulse(BULB_PIN); // Disable library's blink-on-pulse feature
      }
      
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
    

    

  

  // if (buttonState == HIGH){
  //   Serial.println("Button_press");

  //   if (state == 0){

  //     // Motion detection
  //     int motionDetected = digitalRead(MOTION_SENSOR_PIN); // Read motion sensor
  //     if (motionDetected == HIGH)
  //     {
  //       Serial.println("HIGH");
  //       if (pirState == LOW)
  //       {
  //         Serial.println("Motion detected!");
  //         pirState = HIGH;
  //       }

  //       // Read temperature and humidity
  //       DHT.read();
  //       float temperatureC = DHT.getTemperature();
  //       float temperature = (temperatureC * 1.8) + 32; // Convert to Fahrenheit
  //       float humidity = DHT.getHumidity();

  //       // Serial.print("Temperature: ");
  //       // Serial.print(temperature);
  //       // Serial.println(" °F");

  //       // Serial.print("Humidity: ");
  //       // Serial.print(humidity);
  //       // Serial.println(" %");

  //       // Set LED color based on temperature
  //       setLEDColor(temperature);
  //     }
  //     else
  //     {
  //       if (pirState == HIGH)
  //       {
  //         Serial.println("Motion ended!");
  //         pirState = LOW;
  //         // Turn off the LED strip when no motion is detected
  //         fill_solid(leds, NUM_LEDS, CRGB::Black);
  //         FastLED.show();
  //       }
  //     } 

  //     state = 1; 

  //   } else{

  //     Serial.println("Switched to heart rate mode.");
  //     state = 0;

  //   }

    

  // }

  // delay(100);

  // Heartbeat detection
  // if (pulseSensor.sawStartOfBeat())
  // {
  //   Serial.println("Heartbeat detected!");
  //   // Change LED strip behavior for heartbeat
  //   fill_solid(leds, NUM_LEDS, CRGB::Purple);
  //   FastLED.show();
  //   delay(50);
  //   fill_solid(leds, NUM_LEDS, CRGB::Black);
  //   FastLED.show();
  // }

  

  // Heartbeat sensor periodic update
  // if (pulseSensor.UsingHardwareTimer)
  // {
  //   delay(20);
  //   pulseSensor.outputSample();
  // }
  // else
  // {
  //   if (pulseSensor.sawNewSample())
  //   {
  //     if (--pulseSensor.samplesUntilReport == (byte)0)
  //     {
  //       pulseSensor.samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
  //       pulseSensor.outputSample();
  //     }
  //   }
  // }
}