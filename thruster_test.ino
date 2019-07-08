#include "Servo.h" // include servo library

Servo servo;  // create servo object to control a servo

int pwmValue = 1500;

// the setup routine runs once when you press reset:
void setup() {
  
  servo.attach(9);  // attaches the servo on pin 9 to the servo object
  servo.writeMicroseconds(pwmValue); // write PWM value to ESC
  delay(1000);
}

// the loop routine runs over and over again forever:
void loop() {
  //int pwmValue = map(analogRead(A0), 0, 1023, 1100, 1900); // convert analog voltage on A0 to PWM value

  //servo.writeMicroseconds(1500);
  //delay(1000);
  for(int i = 1535; i< 1575; i++){
      servo.writeMicroseconds(i);
      delay(100);
  }
  for( int i = 1575; i> 1535; i--){
      servo.writeMicroseconds(i);
      delay(100);
  }
  
  
}

