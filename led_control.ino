// include light_ws2812 library
#include <cRGB.h>
#include <WS2812.h>
#include <EEPROM.h>

#define PIN 10  // Digital output pin (default: 7)
#define LED_COUNT 30   // Number of LEDs to drive (default: 9)

const uint8_t curve[16] PROGMEM = {
    1, 2, 3, 4, 6, 8, 11, 16, 22, 32, 45, 63, 90, 127, 180, 255
};

enum modes {
    MODE_OFF = 0,
    MODE_BLINK_RGB = 1,
    MODE_COLOR = 2,
    MODE_FADE_COLOR = 3,
    MODE_RUNNING_COLOR = 4
};

enum commands {
    COMMAND_MODE = 0,
    COMMAND_SPEED = 1,
    COMMAND_COLOR = 2
};

WS2812 led(LED_COUNT);
int current_mode;
int current_speed;
cRGB current_color;

void setup() {
    Serial.begin(9600);
    Serial.println("LED Control");
    randomSeed(analogRead(0));
    led.setOutput(PIN);

    current_mode = EEPROM.read(0);
    current_speed = EEPROM.read(1);
    current_color.r = EEPROM.read(2);
    current_color.g = EEPROM.read(3);
    current_color.b = EEPROM.read(4);
    printMode();
}

void loop() {
    if (Serial.available() > 0) {
        readCommand();
        printMode();
    }

    switch (current_mode) {
        case MODE_OFF:
            powerDown();
            break;
        case MODE_BLINK_RGB:
            blinkRGB(current_speed);
            break;
        case MODE_COLOR:
            setAllLED(current_color);
            break;
        case MODE_FADE_COLOR:
            //fadeColor();
            break;
        case MODE_RUNNING_COLOR:
            running_color();
            break;
        default:
            powerDown();
            break;
    }
}

void readCommand() {
    int command = Serial.parseInt();
    switch (command) {
        case COMMAND_MODE:
            Serial.println("Mode?");
            while(!Serial.available());
            current_mode = Serial.parseInt();
            EEPROM.write(0, current_mode);
            Serial.println(current_mode);
            break;
        case COMMAND_SPEED:
            Serial.println("Speed?");
            while(!Serial.available());
            current_speed = Serial.parseInt();
            Serial.println(current_speed);
            EEPROM.write(1, current_speed);
            break;
        case COMMAND_COLOR:
            Serial.println("Color?");
            while(!Serial.available());
            current_color.r = Serial.parseInt();
            EEPROM.write(2, current_color.r);
            Serial.println(current_color.r);
            while(!Serial.available());
            current_color.g = Serial.parseInt();
            EEPROM.write(3, current_color.g);
            Serial.println(current_color.g);
            while(!Serial.available());
            current_color.b = Serial.parseInt();
            Serial.println(current_color.b);
            EEPROM.write(4, current_color.b);
            break;
        default:
            Serial.println("Unknown Command");
            break;
    }
}

void printMode() {
    switch (current_mode) {
        case MODE_OFF:
            Serial.print("MODE: OFF");
            break;
        case MODE_BLINK_RGB:
            Serial.print("MODE: BLINK_RGB");
            break;
        case MODE_COLOR:
            Serial.print("MODE: COLOR");
            break;
        case MODE_RUNNING_COLOR:
            Serial.print("MODE: RUNNING_COLOR");
            break;
        default:
            Serial.print("UNKNOWN MODE: ");
            Serial.print(current_mode);
            break;
    }
    Serial.print(", SPEED: ");
    Serial.print(current_speed);
    Serial.print(", COLOR: R:");
    Serial.print(current_color.r);
    Serial.print(" G:");
    Serial.print(current_color.g);
    Serial.print(" B:");
    Serial.println(current_color.b);
}

void setAllLED(cRGB value) {
    for (int i = 0; i < LED_COUNT; i++) {
        led.set_crgb_at(i, value);
    }
    led.sync();
}

void powerDown() {
    cRGB value;

    value.r = 0;
    value.g = 0;
    value.b = 0;
    setAllLED(value);
}

void blink(cRGB value, int speed, int times) {
    setAllLED(value);
    for (int i = 1; i < times; i++) {
        delay(speed);
        powerDown();
        delay(speed);
        setAllLED(value);
    }
    powerDown();
}

void blinkRGB(int speed) {
    cRGB value;

    value.r = 255;
    value.g = 0;
    value.b = 0;
    setAllLED(value);

    delay(speed);

    value.r = 0;
    value.g = 255;
    value.b = 0;
    setAllLED(value);

    delay(speed);

    value.r = 0;
    value.g = 0;
    value.b = 255;
    setAllLED(value);

    delay(speed);
}

void fadeColor(cRGB color, int speed) {
    float factor_r = color.r / 255;
    float factor_g = color.g / 255;
    float factor_b = color.b / 255;

    uint8_t steps = sizeof(curve) / sizeof(curve[0]);

    cRGB value;
    value.r = 0;
    value.g = 0;
    value.b = 0;

    for (uint8_t i = 0; i < steps; i++) {
        setAllLED(color);

        delay(speed);
    }

    for (uint8_t i = 0; i < steps; i++) {
        setAllLED(color);
        delay(speed);
    }
}

void running_color() {
    for (int i = 0; i < LED_COUNT; i++) {
        cRGB halfValue;
        halfValue.r = current_color.r/16;
        halfValue.g = current_color.g/16;
        halfValue.b = current_color.b/16;
        
        cRGB quarterValue;
        quarterValue.r = current_color.r/32;
        quarterValue.g = current_color.g/32;
        quarterValue.b = current_color.b/32;
        
        powerDown();
        if (i > 2) {
            led.set_crgb_at(i-2, halfValue);
        }
        if (i < LED_COUNT - 2) {
            led.set_crgb_at(i+2, halfValue);
        }
        if (i > 1) {
            led.set_crgb_at(i-1, halfValue);
        }
        if (i < LED_COUNT - 1) {
            led.set_crgb_at(i+1, halfValue);
        }
        delay(1);
        led.set_crgb_at(i, current_color);
        led.sync();
        delay(current_speed);
    }
}

