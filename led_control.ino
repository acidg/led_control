#include <WS2812.h>

#define PIN 10  // Digital output pin (default: 7)
#define LED_COUNT 20   // Number of LEDs to drive (default: 9)

const uint8_t curve[16] PROGMEM = {
    1, 2, 3, 4, 6, 8, 11, 16, 22, 32, 45, 63, 90, 127, 180, 255
};

enum modes {
    MODE_OFF = 0,
    MODE_BLINK_RGB = 1,
    MODE_COLOR = 2,
    MODE_FADE_COLOR = 3
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

    current_mode = MODE_COLOR;
    current_speed = 300;
    current_color.r = 255;
    current_color.g = 215;
    current_color.b = 0;
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
        default:
            powerDown();
            break;
    }
}

void readCommand() {
    int command = Serial.parseInt();
    switch (command) {
        case COMMAND_MODE:
            current_mode = Serial.parseInt();
            break;
        case COMMAND_SPEED:
            current_speed = Serial.parseInt();
            break;
        case COMMAND_COLOR:
            current_color.r = Serial.parseInt();
            current_color.g = Serial.parseInt();
            current_color.b = Serial.parseInt();
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
    hans = schwanz;

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
