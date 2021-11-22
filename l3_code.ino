/*
 * Alden Giedraitis
 * EECE.4520
 * Lab 3: Fan Controller with Time
 * Program Description: The following code drives a circuit
 * to control fan speed and display time on a display.
 */

/*
 * SSD1306 library adapted from AdaFruit https://github.com/adafruit/Adafruit_SSD1306
 * DS3231 library adapted from Andrew Wickert https://github.com/NorthernWidget/DS3231
 * Fan speed code adapted from tylerpeppy https://create.arduino.cc/projecthub/tylerpeppy/25-khz-4-pin-pwm-fan-control-with-arduino-uno-3005a1
 * TCA954A8 code adapted from DroneBot Workshop https://dronebotworkshop.com/multiple-i2c-bus/
 * 
 * All code modified by Alden Giedraitis
 */

 #include "Wire.h"
 #define TCAADDR 0x70 //Address of TCA9548A multiplexer


 //INCLUDED LIBRARIES
 #include <DS3231.h>
 #include <SPI.h>
 #include <Adafruit_SSD1306.h>
 #include <Adafruit_GFX.h>


//MOTOR PROPERTIES
//=============================================================
const int mtrPin = 9;

const word PWM_FREQ_HZ = 25000; //Adjust this value to adjust the frequency (Frequency in HZ!) (Set currently to 25kHZ)
const word TCNT1_TOP = 16000000/(2*PWM_FREQ_HZ);

volatile int fan_mode = 0; //Fan mode for changing speed

String fMode = "0"; //String to hold what to do display for speed

//=============================================================




//CLOCK PROPERTIES
//===========================================================
DS3231 clock;
#define DS3231_I2C_Addr 0x68

byte hour;
byte minute; 
byte second;
//===========================================================



//DISPLAY PROPERTIES
//===========================================================
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display1(OLED_RESET);

bool century = false;
bool h12Flag;
bool pmFlag;

//===========================================================



//Switch to selected I2C address and send data over
void TCA_Select(uint8_t i){
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {

  Serial.begin(9600);

  Wire.begin();

  //Initialize display
  
  TCA_Select(0);
  display1.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

//Initialize motor

pinMode(mtrPin, OUTPUT);

  // Clear Timer1 control and count registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // Set Timer1 configuration
  // COM1A(1:0) = 0b10   (Output A clear rising/set falling)
  // COM1B(1:0) = 0b00   (Output B normal operation)
  // WGM(13:10) = 0b1010 (Phase correct PWM)
  // ICNC1      = 0b0    (Input capture noise canceler disabled)
  // ICES1      = 0b0    (Input capture edge select disabled)
  // CS(12:10)  = 0b001  (Input clock select = clock/1)
  
  TCCR1A |= (1 << COM1A1) | (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << CS10);
  ICR1 = TCNT1_TOP;

  attachInterrupt(0, changeFanSpeed, RISING); //Interrupt to change fan speed (0 is pin 2 on Nano)
  
}

void loop() {
     
      TCA_Select(1); //Switch to RTC address
      hour = clock.getHour(h12Flag,pmFlag);
      minute = clock.getMinute();
      second = clock.getSecond();

      delay(100);


      TCA_Select(0); //Switch to display address

      //Print out time and motor setting

      display1.clearDisplay();

      display1.setTextSize(2);

      display1.setTextColor(WHITE);

      display1.setCursor(0,0);

      display1.print(hour, DEC); //24-hr
      display1.print(":");
      display1.print(minute, DEC);
      display1.print(":");
      display1.println(second, DEC);

      display1.println(fMode);
    

      display1.display();
     
}


void setPwmDuty(byte duty) {
  OCR1A = (word) (duty*TCNT1_TOP)/100;
}


//Change fan speed after button press
void changeFanSpeed(){

  switch(fan_mode){
    //Full RPM
    case 0:
    fMode = "Full";
    setPwmDuty(100);
    fan_mode++;
    break;

    // Set to 3/4 RPM
    case 1:
    fMode = "3/4";
    setPwmDuty(75);
    fan_mode++;
    break;

    // Set to 1/2 RPM
    case 2:
    fMode = "1/2";
    setPwmDuty(50);
    fan_mode++;
    break;

    //Set to 0 RPM
    case 3:
    fMode = "0";
    setPwmDuty(0);
    fan_mode = 0;
    break;
}

}

//Update fan setting on display
void setFanDisp(){

 switch(fan_mode){
    //Full RPM
    case 0:
    fMode = "Full";
    //Serial.println(fMode);
    break;

    // Set to 3/4 RPM
    case 1:
    fMode = "3/4";
    //Serial.println(fMode);
    break;

    // Set to 1/2 RPM
    case 2:
    fMode = "1/2";
    //Serial.println(fMode);
    break;

    //Set to 0 RPM
    case 3:
    fMode = "0";
    //Serial.println(fMode);
    break;
}

}
