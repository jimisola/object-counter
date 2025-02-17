#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define DEBUG_MODE false

// TCRT5000 Sensor pin
#define TCRT500_DIGITAL_SENSOR_PIN 2

// Push button Sensor pin
#define RESET_BUTTON_PIN 3

// Define TFT pins
#define TFT_BACKLIGHT 4
#define TFT_RST 7
#define TFT_DC 8
#define TFT_CS 9
#define TFT_MOSI 11
#define TFT_SCLK 13

// Constants
#define TFT_FRAME_RATE 500
#define TFT_BRIGHTNESS 128
#define TCRT500_RESET_THRESHOLD 2000
#define RESET_DELAY 2000
#define DEBOUNCE_TIME_SENSOR 400
#define DEBOUNCE_PUSH_BUTTON 500

// Global variables
volatile bool initialDisplay = true;
volatile unsigned int counter = 0;
unsigned int objectsPerMinute = 0;
unsigned int objectsPerHour = 0;
unsigned long lastRateUpdate = 0;
unsigned long timerStart = 0;
unsigned int elapsedTimeMinutes = 0;
unsigned int elapsedTimeHours = 0;

volatile bool sensorTriggered = false;
volatile unsigned long sensorTriggerStartTime = 0;

volatile bool doReset = false;

#define LINE1 12
#define LINE2 52
#define LINE3 77
#define LINE4 102

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void resetCounters()
{
  initialDisplay = true;
  counter = 0;
  objectsPerMinute = 0;
  objectsPerHour = 0;
  lastRateUpdate = 0;
  timerStart = 0;
  elapsedTimeMinutes = 0;
  elapsedTimeHours = 0;

  sensorTriggered = false;
  sensorTriggerStartTime = 0;
  timerStart = millis();
}

void displayValues()
{
  static unsigned int lastCounter = 0, lastObjectsPerMinute = 0, lastObjectsPerHour = 0;
  static unsigned int lastElapsedTimeMinutes = 0, lastElapsedTimeHours = 0;

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); // Overwrite previous text with background color

  if (initialDisplay)
  {
    // Draw static labels only once
    tft.setCursor(0, LINE1);
    tft.setTextSize(3);
    tft.print("#: ");
    tft.setTextSize(2);
    tft.setCursor(0, LINE2);
    tft.print("No/m: ");
    tft.setCursor(0, LINE3);
    tft.print("No/h: ");
    tft.setCursor(0, LINE4);
    tft.print("Timer: ");
  }

  // Print dynamic values at correct offsets
  if (initialDisplay || counter != lastCounter)
  {
    tft.fillRect(80, LINE1, 50, 32, ST77XX_WHITE);
    tft.setCursor(80, LINE1);
    tft.setTextSize(3);
    tft.print(counter);
    tft.setTextSize(2);
    lastCounter = counter;
  }

  if (initialDisplay || objectsPerMinute != lastObjectsPerMinute)
  {
    tft.fillRect(80, LINE2, 50, 16, ST77XX_WHITE);
    tft.setCursor(80, LINE2);
    tft.print(objectsPerMinute);
    lastObjectsPerMinute = objectsPerMinute;
  }

  if (initialDisplay || objectsPerHour != lastObjectsPerHour)
  {
    tft.fillRect(80, LINE3, 50, 16, ST77XX_WHITE);
    tft.setCursor(80, LINE3);
    tft.print(objectsPerHour);
    lastObjectsPerHour = objectsPerHour;
  }

  if (initialDisplay || elapsedTimeMinutes != lastElapsedTimeMinutes || elapsedTimeHours != lastElapsedTimeHours)
  {
    tft.fillRect(80, LINE4, 100, 16, ST77XX_WHITE);
    tft.setCursor(80, LINE4);
    tft.print(elapsedTimeMinutes > 9 ? "" : "0");
    tft.print(elapsedTimeMinutes);
    tft.print(":");
    tft.print(elapsedTimeHours > 9 ? "" : "0");
    tft.print(elapsedTimeHours);
    lastElapsedTimeMinutes = elapsedTimeMinutes;
    lastElapsedTimeHours = elapsedTimeHours;
  }

  initialDisplay = false;
}

void sensorChanged()
{
  static bool lastSensorState = HIGH;
  static bool sensorTriggered = false;
  static unsigned long lastTriggerTime = 0;
  unsigned long currentTime = millis();

  if (DEBUG_MODE)
    Serial.println("### inSensorChanged");

  bool currentSensorState = digitalRead(TCRT500_DIGITAL_SENSOR_PIN);

  if (currentSensorState == LOW && lastSensorState == HIGH) // FALLING edge detected
  {
    if (DEBUG_MODE)
      Serial.println("### Sensor changed to LOW");

    if (!sensorTriggered && currentTime - lastTriggerTime >= DEBOUNCE_TIME_SENSOR) // First detection
    {
      sensorTriggered = true;
      lastTriggerTime = currentTime;
      counter++;
      if (DEBUG_MODE)
        Serial.println("### Counter increased: " + String(counter));
    }
  }
  else if (currentSensorState == HIGH && lastSensorState == LOW) // RISING edge detected
  {
    if (DEBUG_MODE)
      Serial.println("### Sensor changed to HIGH");

    if (sensorTriggered)
    {
      // If the sensor was held for TCRT500_RESET_THRESHOLD, request a reset
      if (currentTime - lastTriggerTime >= TCRT500_RESET_THRESHOLD)
      {
        if (DEBUG_MODE)
          Serial.println("doReset in sensorChanged");
        doReset = true;
      }
    }

    sensorTriggered = false;
  }
  else
  {
    if (DEBUG_MODE)
      Serial.println("### Sensor state did not changed");
  }

  lastSensorState = currentSensorState;
}

void buttonPushed()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  if (last_interrupt_time != 0 && (interrupt_time - last_interrupt_time > DEBOUNCE_PUSH_BUTTON))
  {
    Serial.println("doReset in buttonPushed");
    doReset = true;
  }

  last_interrupt_time = interrupt_time;
}

void reset()
{
  Serial.println("Resetting counters...");

  tft.fillScreen(ST77XX_WHITE);
  tft.setCursor(0, LINE2);
  tft.println("System reset...");

  unsigned long resetStartTime = millis();
  while (millis() - resetStartTime < RESET_DELAY)
  {
    // Wait without blocking interrupts
  }

  resetCounters();
  tft.fillScreen(ST77XX_WHITE);
  displayValues();
}

void setBrightness(int level)
{
  analogWrite(TFT_BACKLIGHT, constrain(level, 0, 255));
}

void setup()
{
  Serial.begin(115200);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
  tft.setTextSize(1);

  pinMode(TFT_BACKLIGHT, OUTPUT);
  setBrightness(TFT_BRIGHTNESS);

  tft.setCursor(0, 0);
  tft.print("To reset: press button or trigger sensor (" + String(2000 / 1000) + "s)");
  tft.setCursor(0, 120);
  tft.print("version 0.1.0 by jimisola");

  delay(3000);
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextSize(2);
  displayValues();

  timerStart = millis();

  // configure tcrt500 sensor pin
  pinMode(TCRT500_DIGITAL_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TCRT500_DIGITAL_SENSOR_PIN), sensorChanged, CHANGE);

  // configure reset button interrupt
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN), buttonPushed, FALLING);
}

void loop()
{
  unsigned long currentTime = millis();

  if (doReset)
  {
    reset();
    doReset = false;
  }

  if (currentTime - lastRateUpdate >= TFT_FRAME_RATE)
  {
    float elapsedMinutes = (currentTime - timerStart) / 60000.0;
    float elapsedHours = elapsedMinutes / 60.0;

    objectsPerMinute = (elapsedMinutes > 0) ? (counter / elapsedMinutes) : 0;
    objectsPerHour = (elapsedHours > 0) ? (counter / elapsedHours) : 0;

    elapsedTimeMinutes = (currentTime - timerStart) / 60000;
    elapsedTimeHours = elapsedTimeMinutes / 60;
    elapsedTimeMinutes %= 60;

    lastRateUpdate = currentTime;
    displayValues();
  }
}
