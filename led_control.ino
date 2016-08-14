#include <Arduino.h>

#include <WS2812.h>

#define PIN 10  // Digital output pin (default: 7)
#define LED_COUNT 10   // Number of LEDs to drive (default: 9)

WS2812 led(LED_COUNT); 

void setup() {
    Serial.begin(9600);
    Serial.println("LED Control");
    randomSeed(analogRead(0));
    led.setOutput(PIN);
}

void loop() {
    blinkRGB(300);
    delay(10000);
    Serial.println("blubb");
}

void blinkRGB(int speed) {
    cRGB value;
    value.r = 255;
    value.g = 0;
    value.b = 0;
    for (int i = 0; i < LED_COUNT; i++) {
        led.set_crgb_at(i, value);
    }
    led.sync();

    delay(speed);
    
    value.r = 0;
    value.g = 255;
    value.b = 0;
    for (int i = 0; i < LED_COUNT; i++) {
        led.set_crgb_at(i, value);
    }
    led.sync();

    delay(speed);
    
    value.r = 0;
    value.g = 0;
    value.b = 255;
    for (int i = 0; i < LED_COUNT; i++) {
        led.set_crgb_at(i, value);
    }
    led.sync();

    delay(speed);
}


