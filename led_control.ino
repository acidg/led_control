// include light_ws2812 library
#include <cRGB.h>
#include <WS2812.h>
#include <EEPROM.h>
#include <BMA020.h>

#define PIN 9  // Digital output pin (default: 7)
#define LED_COUNT 30   // Number of LEDs to drive (default: 9)

const uint8_t curve[16] PROGMEM = {
    1, 2, 3, 4, 6, 8, 11, 16, 22, 32, 45, 63, 90, 127, 180, 255
};

/* eprom save locations */
enum eprom {
    EPROM_MODE = 0,
    EPROM_SPEED = 1,
    EPROM_R = 2,
    EPROM_G = 3,
    EPROM_B = 4
};

/* Available patterns */
enum modes {
    MODE_OFF = 0,
    MODE_BLINK_RGB = 1,
    MODE_COLOR = 2,
    MODE_FADE_COLOR = 3,
    MODE_RUNNING_COLOR = 4,
    MODE_RUNNING_PARALLEL = 5,
    MODE_LEFT_RIGHT_TEST = 6
};

/* The three main commands for adjusting the mode, speed or color */
enum commands {
    COMMAND_MODE = 0,
    COMMAND_SPEED = 1,
    COMMAND_COLOR = 2
};

/* Management datastructure? */
WS2812 led(LED_COUNT);

/* self explanatory */
int current_mode;
int current_speed;
cRGB current_color;

/* Executed once during Arduino startup */
void setup() {
    // Starte I2C verbindung und setzte Messbereich auf 2g
    BMA::begin(RANGE_2g, BANDWIDTH_25hz); 

    Serial.begin(9600);
    Serial.println("LED Control");
    randomSeed(analogRead(0));
    led.setOutput(PIN);

    current_mode = EEPROM.read(EPROM_MODE);
    current_speed = EEPROM.read(EPROM_SPEED);
    current_color.r = EEPROM.read(EPROM_R);
    current_color.g = EEPROM.read(EPROM_G);
    current_color.b = EEPROM.read(EPROM_B);
    printMode();
}

/* Executed continuously after succesful startup */
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
            fadeColor(current_color, current_speed);
            break;
        case MODE_RUNNING_COLOR:
            running_color();
            break;
        case MODE_RUNNING_PARALLEL:
            running_parallel();
            break;
        case MODE_LEFT_RIGHT_TEST:
            left_right_test();
            break;
        default:
            powerDown();
            break;
    }
}

int read_serial() {
    while(!Serial.available());
    int tmp = Serial.parseInt();
    Serial.print(tmp);
    return tmp;
}

void readCommand() {
    Serial.print("Change Mode(0), Speed(1) or Color(2)? ");
    int command = read_serial();
    Serial.println();

    switch (command) {
        case COMMAND_MODE:
            Serial.print("Mode? ");
            current_mode = read_serial();
            EEPROM.write(EPROM_MODE, current_mode);
            Serial.println();
            break;
        case COMMAND_SPEED:
            Serial.print("Speed? ");
            current_speed = read_serial();
            EEPROM.write(EPROM_SPEED, current_speed);
            Serial.println();
            break;
        case COMMAND_COLOR:
            Serial.print("Color? ");

            Serial.print("r: ");
            current_color.r = read_serial();
            EEPROM.write(EPROM_R, current_color.r);

            Serial.print(" g: ");
            current_color.g = read_serial();
            EEPROM.write(EPROM_G, current_color.g);

            Serial.print(" b: ");
            current_color.b = read_serial();
            EEPROM.write(EPROM_B, current_color.b);
            Serial.println();
            break;
        default:
            Serial.println("Unknown Command");
            break;
    }
}

void printMode() {
    Serial.print("Mode: ");
    switch (current_mode) {
        case MODE_OFF:
            Serial.print("OFF");
            break;
        case MODE_BLINK_RGB:
            Serial.print("BLINK_RGB");
            break;
        case MODE_COLOR:
            Serial.print("COLOR");
            break;
        case MODE_RUNNING_COLOR:
            Serial.print("RUNNING_COLOR");
            break;
        case MODE_LEFT_RIGHT_TEST:
            Serial.print("LEFT_RIGHT_TST");
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
    cRGB halfValue;
    halfValue.r = current_color.r/16;
    halfValue.g = current_color.g/16;
    halfValue.b = current_color.b/16;
    
    cRGB quarterValue;
    quarterValue.r = current_color.r/32;
    quarterValue.g = current_color.g/32;
    quarterValue.b = current_color.b/32;

    for (int i = 0; i < LED_COUNT; i++) {
        powerDown();
        if (i > 2) led.set_crgb_at(i-2, halfValue);
        if (i > 1) led.set_crgb_at(i-1, halfValue);
        if (i < LED_COUNT - 2) led.set_crgb_at(i+2, halfValue);
        if (i < LED_COUNT - 1) led.set_crgb_at(i+1, halfValue);
        delay(1);
        led.set_crgb_at(i, current_color);
        led.sync();
        delay(current_speed);
    }
}

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

/* lights running along the two stripes (15 LEDs each) at the same time - should hopefully look cool under the cruiser */
void running_parallel() {
    cRGB halfValue;
    halfValue.r = current_color.r/16;
    halfValue.g = current_color.g/16;
    halfValue.b = current_color.b/16;
    
    cRGB quarterValue;
    quarterValue.r = current_color.r/32;
    quarterValue.g = current_color.g/32;
    quarterValue.b = current_color.b/32;

    int led_count_half = LED_COUNT / 2;

    // have to go through from 0-14 and 29-15 (when it's 30 LEDs, otherwise adapt values accordingly)
    for (int i = 0; i < led_count_half; i++) {
        powerDown();

        // first side (0-14)
        led.set_crgb_at(mod(i-2, led_count_half), halfValue);
        led.set_crgb_at(mod(i-1, led_count_half), halfValue);
        led.set_crgb_at(i, current_color);
        led.set_crgb_at(mod(i+1, led_count_half), halfValue);
        led.set_crgb_at(mod(i+2, led_count_half), halfValue);

        // second side (29-15)
        led.set_crgb_at(LED_COUNT-1 - mod(i-2, led_count_half), halfValue);
        led.set_crgb_at(LED_COUNT-1 - mod(i-1, led_count_half), halfValue);
        led.set_crgb_at(LED_COUNT-1 - i, current_color);
        led.set_crgb_at(LED_COUNT-1 - mod(i+1, led_count_half), halfValue);
        led.set_crgb_at(LED_COUNT-1 - mod(i+2, led_count_half), halfValue);

        delay(1);
        led.sync();
        delay(current_speed);
    }
}

void setLEDsInRange(cRGB color, int start, int stop) {
    for (int i = start; i < stop; i++) {
        led.set_crgb_at(i, color);
    }
    led.sync();
}

void left_right_test() {
    int middle_treshold = 40;
    int manni_treshold = 30;

    cRGB side1, side2;

    BMA::read_all_axes();

    if (BMA::Y > manni_treshold) {
        //side 1 = pink
        side1.r = 255;
        side1.g = 20;
        side1.b = 150;

        //side 2 = pink
        side2.r = 255;
        side2.g = 20;
        side2.b = 150;
    } else {
        if (BMA::X < -middle_treshold) {
            // side 1 = scaled red
            side1.r = -BMA::X;
            side1.g = 0;
            side1.b = 0;

            // side 2 = off
            side2.r = 0;
            side2.g = 0;
            side2.b = 0;

        } else if (BMA::X > middle_treshold) {
            // side 1 = off
            side1.r = 0;
            side1.g = 0;
            side1.b = 0;

            // side 2 = scaled green
            side2.r = 0;
            side2.g = BMA::X;
            side2.b = 0;
        } else {
            // side 1 = blue
            side1.r = 0;
            side1.g = 0;
            side1.b = 255;

            // side 2 = blue
            side2.r = 0;
            side2.g = 0;
            side2.b = 255;
        }
    }

    // side 1: 0 - 14
    setLEDsInRange(side1, 0, 14);
    // side 2: 29 - 15
    setLEDsInRange(side2, 15, 29);
}
