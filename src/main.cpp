#include "Arduino.h"
#include <ESP32Servo.h>
#include "config.h"
#include "Reciever.h"
#include "FlightController.h"
// ---------------------------------------------------------------------------
//Globals
 double pastRh = 0;
 double valueRh = 0;

 double pastRv = 0;
 double valueRv = 0;

 double pastLv = 0;
 double valueLv = 0;

 double pastLh = 0;
 double valueLh = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

//Reciever/comms
Reciever* rc = new Reciever(&valueLh, &valueLv, &valueRh, &valueRv);
//Motor system controls
FlightController* flight;
Servo motA, motB, motC, motD;
Servo motors[4]; 
// ---------------------------------------------------------------------------


//----------------------------------------------------------------------------
//ISR Routines
void changeRh(){
  portENTER_CRITICAL_ISR(&mux);
  double temp = esp_timer_get_time()-pastRh;
  if(digitalRead(25) == LOW){
	if(temp > 950 && temp < 2100){
		valueRh = temp;
  	}
  }else{
	pastRh = esp_timer_get_time();	
  }
  portEXIT_CRITICAL_ISR(&mux);
}
void changeRv(){
  portENTER_CRITICAL_ISR(&mux);
  if(digitalRead(26) == LOW){
  	double temp = micros()-pastRv;
  	if(temp > 950 && temp < 2100){
		  valueRv = temp;
  	}else{
      valueRv = temp;
  	}
  }
  else{
  	pastRv = micros();
  }
  portEXIT_CRITICAL_ISR(&mux);
}
void changeLh(){
  portENTER_CRITICAL_ISR(&mux);
  double temp = esp_timer_get_time() - pastLh;
  if(digitalRead(27) == LOW){
    if(temp > 950 && temp < 2100){
	    valueLh = temp;
    }
  }
  else{
     pastLh = esp_timer_get_time();
  }
  portEXIT_CRITICAL_ISR(&mux);
}
void changeLv(){
  portENTER_CRITICAL_ISR(&mux);
  double temp = esp_timer_get_time() - pastLv;
  if(digitalRead(14) == LOW){
    if(temp > 950 && temp < 2100){
	    valueLv = temp;
    }
  }else{
    pastLv = esp_timer_get_time();
  }
  portEXIT_CRITICAL_ISR(&mux);
}
//---------------------------------------------------------------------------

/**
 * Start serial, attach motors, and display the instructions and format
 */
void setup() {
    Serial.begin(115200);
    motA.attach(UPPER_LEFT_MOTOR, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);
    motB.attach(UPPER_RIGHT_MOTOR, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);
    motC.attach(LOWER_LEFT_MOTOR, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);
    motD.attach(LOWER_RIGHT_MOTOR, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);
   
    motors[0]= motA;
    motors[1]= motB;
    motors[2]= motC;
    motors[3]= motD;
    flight = new FlightController(rc, motors);

    //Attach interrupts for the reciever
    pinMode(25, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(25), changeRh, CHANGE);
    pinMode(26, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(26), changeRv, CHANGE);
    pinMode(27, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(27), changeLh, CHANGE);
    pinMode(14, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(14), changeLv, CHANGE);

    //Start up ESC's 
    motA.writeMicroseconds(2000); 
    motB.writeMicroseconds(2000); 
    motC.writeMicroseconds(2000); 
    motD.writeMicroseconds(2000); 
    delay(2000);
    motA.writeMicroseconds(1000); 
    motB.writeMicroseconds(1000); 
    motC.writeMicroseconds(1000); 
    motD.writeMicroseconds(1000); 
    delay(1000);
}


/**
 * Loop: Read input and execute instruction
 */
void loop() {
	flight->loop();
	rc->print();
}
