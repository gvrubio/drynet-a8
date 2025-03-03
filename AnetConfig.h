//// LCD
// CONNECTOR
#define EXP1_01_PIN 17  // BEEPER / ENC X
#define EXP1_02_PIN 30  // LCD_D4 / SERVO
#define EXP1_03_PIN 16  // ENC    / LCD_EN X
#define EXP1_04_PIN 29  // SERVO  / LCD_RS X
#define EXP1_05_PIN 11  // EN1    / LCD_D4 X
#define EXP1_06_PIN 28  // LCD_EN / EN1 X
#define EXP1_07_PIN 10  // EN2 X
#define EXP1_08_PIN 27  // LCD_RS / BEEPER

// LCD REWRITE
#define SERVO0_PIN 27   //A4
#define LCD_PINS_RS 28  //A3
#define LCD_PINS_EN 29  //A2
#define LCD_PINS_D4 10
#define LCD_PINS_D5 11
#define LCD_PINS_D6 16
#define LCD_PINS_D7 17

// KEYPAD
#define KEYPAD A6

// ANALOG INs
#define TBED A0
#define TEND A1

//DIGITAL OUTS
#define HOTEND 13
#define HOTBED 12

//FAN
#define FAN1 4
#define FAN2 HOTEND

//DIGITAL INS
#define X_STOP 18
#define Y_STOP 19
#define Z_STOP 20

//STEPPERS
#define Z_ENABLE A2  //?
#define Z_STEP 3
#define Z_DIR 2
#define XYE_ENABLE 14
#define X_STEP 15
#define X_DIR 21
#define Y_STEP 22
#define Y_DIR 23
#define E_STEP 1
#define E_DIR 0

//I2C REWIRE
#define PIN_WIRE_SDA 5
#define PIN_WIRE_SCL 6
