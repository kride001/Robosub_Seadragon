#include <Wire.h>
#include "MS5837.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define PIN            8  // LED PIN
#define NUMPIXELS      9
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval 	= 500;	// Delay for half a second
int j 		= 0;	// Used in case 3 led state
int k 		= 0;	// Blinking led
int m 		= 0; 
int rvsReady 	= 0;
int sqnReady 	= 0;
int prevTime 	= 0;

int solenoid_input 	= 3;	// Pin number (9 on the schematic)
const int highMask 	= 0xF0;
const int lowMask 	= 0x0F;
int solenoid_signal 	= 0;
int fired 		= 0;
int led_signal 		= 0;

MS5837 sensor;

//------------------ Classes -------------------------------------------
class Led_Class {
  private:
  public:
  //Member functions
  void update_led(int state);
};

class Solenoid{
  private:
  int SSignal; 		// Signal used by arduino to control flow of voltage foing into the solinoid
  
  public:  
   Solenoid(int);	// Constructor to create a object from the input solenoid signal
   Solenoid Drop(); 	// Activate solenoid to drop the marketn when fixed on a target
   
};
//------------------ END Classes ------------------------------------------

//------------------ Class Variable Initiaization -------------------------
Solenoid Sole(solenoid_input);
Led_Class led1;
int STATE;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("Starting");
  Wire.begin();
  
 /*while (!sensor.init()) {
    Serial.println("Init failed!");
    Serial.println("Are SDA/SCL connected correctly?");
    Serial.println("Blue Robotics Bar30: White=SDA, Green=SCL");
    Serial.println("\n\n\n");
    delay(5000);
  }*/
  
  sensor.init();
  sensor.setFluidDensity(997); // kg/m^3 (997 freshwater, 1029 for seawater)
  sensor.setModel(MS5837::MS5837_30BA);

  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
   
  pixels.begin();	// This initializes the NeoPixel library.
  strip.begin();
  strip.show();		// Initialize all pixels to 'off'
}
//------------------ END Class Variable Initiaization -------------------------


//-----------------------------------------------------------------------------------------------//
//------------------------------------ MAIN LOOP ------------------------------------------------//

void loop() {
  
  // Read State and Solenoid Command from stm32
  if (Serial1.available()){
    
    STATE = Serial1.read();    // everything received from the stm32 in STATE and solenoid_signal
    
    solenoid_signal = STATE; 
    led_signal = STATE;
    led_signal &= highMask;
    led_signal = led_signal >> 4;
  }
 
  // Read depth sensor data
  sensor.read();
  int depth = 0;
  depth =  sensor.depth() * 39.37;
  Serial.println(sensor.depth());

  // Relay depth information to stm32
  if (depth >= 0){
    Serial1.write(depth);
  }  
  
  // Actuate Led Strip
  if(led_signal == 0 || led_signal == 1 || led_signal == 2 || led_signal == 3 || led_signal == 4){
    led1.update_led(led_signal);
  }
  
 /* solenoid_signal &= lowMask; //only upper 4 bits are kept
    if(solenoid_signal && !fired){        
      // Sole.Drop();
       fired = 1;
     }
     else if(fired && !solenoid_signal){
      fired = 0;
     }
  */

}

//-----------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------//


//------------- Pressure Sensor Functions -----------------------

void print_values(){
  int temp = 0;
  temp = sensor.temperature();
  Serial.print("Pressure: "); 
  Serial.print(sensor.pressure()); 
  Serial.println(" mbar");
  
  Serial.print("Temperature: "); // truncate the double read to an int
  Serial.print(temp); 
  Serial.println(" deg C");
  
  Serial.print("Depth: "); 
  Serial.print(Convert_mtoin(sensor.depth())); 
  Serial.println(" in");
  
  Serial.print("Altitude: "); 
  Serial.print(sensor.altitude()); 
  Serial.println(" m above mean sea level");
  
  delay(400);
}

double Convert_mtoin(double m){
  double inch = m * 39.37;
  return inch;
}
//------------- END Pressure Sensor Functions -----------------


  //------------COMMUNICATION TEST-----------
  /*for(int i = 0; i < 255; i++){
    Serial.print(i);
    Serial.println();
    
    Serial1.write(i);
    delay(200);
  }
  */

//------------- Solenoid Functions ---------------------------

Solenoid::Solenoid(int S){  
  SSignal = S; // assgin pin value inputed to class varaiable
  pinMode(SSignal, OUTPUT);// set pin as a output  
}

Solenoid Solenoid::Drop(){
    // put your main code here, to run repeatedly:  
  digitalWrite(SSignal, HIGH);    //Switch Solenoid ON
  delay(25);                 
  digitalWrite(SSignal, LOW);          //Switch it solenoid off again
  delay(25);
}
//------------- END Solenoid Functions ---------------------------


//--------------- LED Functions -----------------------------------

//enum LED_States {RESET_1};
//Member function definitions
void Led_Class :: update_led(int state){ 
  //===== Actions =====
  switch(state){
    case 0: //ALL Yellow
      for(int i=0;i < NUMPIXELS;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(204,0,204)); // Moderately yellow color.
        pixels.show(); // This sends the updated pixel color to the hardware.
        //delay(10); // Delay for a period of time (in milliseconds).
      } 
    break;
    
    case 1: //ALL RED
      for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(255,3,3)); // Red color.
        pixels.show(); // This sends the updated pixel color to the hardware.
      } 
    break;
    
    case 2: //ALL GREEN
      for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(3,255,3)); // Green color. 3, 255, 3
        pixels.show(); // This sends the updated pixel color to the hardware.
      } 
    break;
    
    case 3: //Cycle GREEN forward and backwards
      //Reset_LED();
      //forward
      if(!rvsReady){
        forwardCycle_3();
        j++;
       }
      
      //backwards
      if(rvsReady){
        backwardCycle_3();
        j--;
       }
      //Reset_LED();
    break;
    
    case 4: //Blue and Gold Rotate    blue : 45, 108, 192   gold: 241,171,0
      if(!sqnReady){
        fwd_full();
        m++;
      }
      if(sqnReady){
        backward_full();
        m--;
      }
      
    break;
    
    case 5:  
      Reset_LED();
    break;
  }
  return;
}

void Reset_LED (){
  for(int i = 0; i < NUMPIXELS; i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    //pixels.show();
    //delay(500);
  }
}

void forwardCycle_3(){
  //for(int j = 0; j < NUMPIXELS ; j++){  
  if(j == 0){
    Reset_LED();
  }
  if(j < NUMPIXELS){
      pixels.setPixelColor(j, pixels.Color(3,255,3));
      if(j >= 3){
        pixels.setPixelColor(j - 3, pixels.Color(0,0,0)); // only 3 leds on
      }
      pixels.show();
      //delay(50);
      prevTime = millis();
     while((millis()- prevTime) < 50){
 
     }
  }
   //}
   
   if(j >= NUMPIXELS){
    pixels.setPixelColor(NUMPIXELS - 2, pixels.Color(0,0,0)); //last 2 pixels stay on, so these lines turn those off
    pixels.show();
    //delay(50);
    prevTime = millis();
    while((millis()- prevTime) < 50){
 
    }
    pixels.setPixelColor(NUMPIXELS - 1, pixels.Color(0,0,0));
    pixels.show();
    //delay(50);
    prevTime = millis();
    while((millis()- prevTime) < 50){
 
    }
    rvsReady  = 1; //READY STATEMENT CHANGE
    //Reset_LED();
  }
}

void backwardCycle_3(){
  //for(int j = NUMPIXELS - 1; j >= 0; j--){
      if(j >= 0){
        pixels.setPixelColor(j, pixels.Color(3,255,3));
        if(j <= NUMPIXELS - 4){
           pixels.setPixelColor(j + 3, pixels.Color(0,0,0));
        }
        pixels.show();
        //delay(50);
        prevTime = millis();
        while((millis()- prevTime) < 50){
 
        }
      //}
      }
      if(j < 0){
        pixels.setPixelColor(2, pixels.Color(0,0,0));
        pixels.show();
        //delay(50);
        prevTime = millis();
        while((millis()- prevTime) < 50){
 
        }
        pixels.setPixelColor(1, pixels.Color(0,0,0));
        pixels.show();
        //delay(50);
        prevTime = millis();
        while((millis()- prevTime) < 50){
  
        }
        pixels.setPixelColor(0, pixels.Color(0,0,0));
        pixels.show();
        //delay(50);
        prevTime = millis();
        while((millis()- prevTime) < 50){
 
        }
        rvsReady = 0; //READY STATEMENT OFF
        //Reset_LED();
      }
}

void fwd_full(){
  //for(int i=0; i < NUMPIXELS; i++){
    
        pixels.setPixelColor(m, pixels.Color(45,108,192)); // UCR Blue 45,108,192
        pixels.show(); 
        prevTime = millis();
        while((millis()- prevTime) < 100){
        }
        if(m >= NUMPIXELS){
          sqnReady = 1;
        }
      //}
}

void backward_full(){
  //for(int i=0; i < NUMPIXELS; i++){
        pixels.setPixelColor(m, pixels.Color(241,171,0)); // UCR Gold 241,171,0
        pixels.show();
        prevTime = millis();
        while((millis()- prevTime) < 100){
        }
        if(m < 0){
          sqnReady = 0;
        }
      //}       
}

void BlinkYellow(){
  if(k == 0){
    for(int i=0; i < NUMPIXELS; i++){
        pixels.setPixelColor(i, pixels.Color(248,255,3)); // yellow color.
        pixels.show(); 
     } 
  
     k = 1;
  }
  else if (k == 1){
    Reset_LED();
    pixels.show(); 
    k = 0;
  }
  //delay(700);
  prevTime = millis();
  while((millis()- prevTime) < 700){
 
  }    
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
//--------------- END LED Functions -----------------------------------


  
