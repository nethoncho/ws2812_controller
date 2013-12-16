#include "FastSPI_LED2.h"

/*
** Parts from sparkfun.com and adafruit.com
**
** RedBoard https://www.sparkfun.com/products/11575
** Joystick Shield Kit https://www.sparkfun.com/products/9760
** Slide Pot https://www.sparkfun.com/products/11620
** SPDT Slide Switch https://www.sparkfun.com/products/9609
** USB-B Mini https://www.sparkfun.com/products/11301
** Addressable RGB LED Strip https://www.sparkfun.com/products/12025
** Power Adapter http://www.adafruit.com/product/368
** 5V 4A Power Supply http://www.adafruit.com/product/1466
** 2 Pin JST Female http://www.adafruit.com/product/318
** 2 Pin JST Male http://www.adafruit.com/product/319
**
** The slide pot goes to A2 (joystick on A0 and A1)
** The toggle switch goes on D7
**
** Light Sequencer http://www.vixenlights.com/
** Using generic serial with a header char of !
**
**
*/

// IO Pins
#define JSTICK_X A0
#define JSTICK_Y A1
#define JSTICK_BTN 2
#define DIMMER A2
#define BUTTON_UP 3
#define BUTTON_RIGHT 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define TOGGLE_SWITCH 7

// Joystick Offset
#define CENTER_X 512
#define CENTER_Y 482

//
#define MASTER_TIMEOUT 2000

// How many leds are in the strip?
#define NUM_LEDS 60

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

// Globals
long Stick_X_Raw = 0;
long Stick_Y_Raw = 0;
float Radius = 0.0;
float Angle = 0.0;
unsigned char Hue = 0;
unsigned char Sat = 0;
int dimmerValue = 255;
unsigned long LastSerialEventTime;
char header = ' ';

void setup()
{

  LastSerialEventTime = millis() + MASTER_TIMEOUT;

  pinMode(JSTICK_BTN, INPUT);
  digitalWrite(JSTICK_BTN, HIGH);
  pinMode(BUTTON_UP, INPUT);
  digitalWrite(BUTTON_UP, HIGH);
  pinMode(BUTTON_RIGHT, INPUT);
  digitalWrite(BUTTON_RIGHT, HIGH);
  pinMode(BUTTON_DOWN, INPUT);
  digitalWrite(BUTTON_DOWN, HIGH);
  pinMode(BUTTON_LEFT, INPUT);
  digitalWrite(BUTTON_LEFT, HIGH);
  pinMode(TOGGLE_SWITCH, INPUT);
  digitalWrite(TOGGLE_SWITCH, HIGH);

  LEDS.addLeds<WS2812, 11, GRB>(leds, NUM_LEDS);

  Serial.begin(115200);
  Serial.setTimeout(20);

  leds[0] = CRGB::Red; 
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Green;
  leds[3] = CRGB::Blue;
  leds[4] = CRGB::Blue;
  leds[5] = CRGB::Blue;
  FastLED.show();
  delay(10);
  fill_solid(leds, NUM_LEDS, CHSV( 0, 0, 0));
  FastLED.show();
}

void loop()
{
  dimmerValue = map(analogRead(DIMMER), 0, 1023, 0, 255);

  if((millis() - LastSerialEventTime) > MASTER_TIMEOUT)
  {
    LocalShowLoop();
    LEDS.setBrightness(dimmerValue);
    FastLED.show();
  }
  else
  {
    LEDS.setBrightness(dimmerValue);
  }
}

void serialEvent()
{
  LastSerialEventTime = millis();
  while(Serial.available())
  {
    header = Serial.read();
    if(header == '!')
    {
      Serial.readBytes((char*)leds, NUM_LEDS * 3);
      FastLED.show();
    }
  }
}

void LocalShowLoop()
{
  CRGB MyRGB[1];

  Stick_X_Raw = analogRead(JSTICK_X) - CENTER_X;
  Stick_Y_Raw = analogRead(JSTICK_Y) - CENTER_Y;
  Radius = (sqrt(sq(Stick_X_Raw)+ sq(Stick_Y_Raw)) / 2.0);
  Angle = atan2((float)Stick_Y_Raw, (float)Stick_X_Raw) * RAD_TO_DEG;
  if(Angle < 0.0)
  {
    Angle = 360.0 - (Angle * -1.0);
  }

  Hue = (unsigned char)(Angle * (255.0 / 360.0));

  if(Radius >= 255.0)
  {
    Sat = 255;
  }
  else
  {
    Sat = Radius;
  }
  
  MyHSVtoRGB(Angle, Sat, 255, MyRGB[0]);
  if(digitalRead(TOGGLE_SWITCH) == LOW)
  {
    fill_solid(leds, NUM_LEDS, MyRGB[0]);
  }
}

void MyHSVtoRGB(uint16_t h, uint8_t s, uint8_t v, struct CRGB & rgb)
{
  uint8_t base;

  if(s == 0)
  {
    rgb.r = v;
    rgb.g = v;
    rgb.b = v;
    return;
  }

  base = ((255 - s) * v) >> 8;

  switch(h/60)
  {
      case 0:
        rgb.r = v;
        rgb.g = (((v - base) * h) / 60) + base;
        rgb.b = base;
        break;
      case 1:
        rgb.r = (((v - base) * (60 - (h % 60))) / 60) + base;
        rgb.g = v;
        rgb.b = base;
        break;
      case 2:
        rgb.r = base;
        rgb.g = v;
        rgb.b = (((v - base) * (h % 60)) / 60) + base;
        break;
      case 3:
        rgb.r = base;
        rgb.g = (((v - base) * (60 - (h % 60))) / 60) + base;
        rgb.b = v;
        break;
      case 4:
        rgb.r = (((v - base) * (h % 60)) / 60) + base;
        rgb.g = base;
        rgb.b = v;
        break;
      default:
        rgb.r = v;
        rgb.g = base;
        rgb.b = (((v - base) * (60 - (h % 60))) / 60) + base;
        break;
  }
}

