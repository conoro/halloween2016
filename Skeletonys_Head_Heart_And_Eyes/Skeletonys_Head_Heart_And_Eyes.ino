// Copyright 2016 Conor O'Neill <conor@conoroneill.com>
// Apache License Version 2.0. See LICENSE file
// Most Servo Sweep code by BARRAGAN <http://barraganstudio.com> and Scott Fitzgerald http://www.arduino.cc/en/Tutorial/Sweep
// Heart pulse code by Sparkfun https://www.sparkfun.com/tutorials/329

#include <Servo.h>

const int buttonPin = 2;
const int ledPin1 = 10;
const int ledPin2 = 11;
const int heartPin = 6;
const int servoPin = 9;
unsigned long startTime;
unsigned long currentTime;

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position
int buttonState = 0; 
int lookingLeft = 0;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(heartPin, OUTPUT);
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  analogWrite(heartPin, 0);

  myservo.attach(servoPin);  // attaches the servo on pin 9 to the servo object
  myservo.write(90);              // tell servo to go to position in variable 'pos' 
  delay(1000);
  myservo.detach();  
}

void loop() {
  float heartIn, heartOut;

  buttonState = digitalRead(buttonPin);

  if (lookingLeft == 0) {
    if (buttonState == HIGH) {
        digitalWrite(ledPin1, HIGH);
        digitalWrite(ledPin2, HIGH);
        myservo.attach(servoPin);  // attaches the servo on pin 9 to the servo object
        myservo.write(180);              // tell servo to go to position in variable 'pos'
        delay(1500);
        myservo.detach();  

        // Check time
        startTime = millis();
      
        lookingLeft = 1;
    }
  } else {
     // Pulse Heart
     for (heartIn = 0; heartIn < 6.283; heartIn = heartIn + 0.001)
     {
       heartOut = sin(heartIn) * 127.5 + 127.5;
       analogWrite(heartPin, heartOut);
     }
     
     // Check time elapsed
     currentTime = millis();
      
     // If 10 seconds passed, look straight again else exit and loop
     if (currentTime - startTime >= 10000){
        digitalWrite(ledPin1, LOW);
        digitalWrite(ledPin2, LOW);
        analogWrite(heartPin, 0);
        myservo.attach(servoPin);  // attaches the servo on pin 9 to the servo object
        myservo.write(90);              // tell servo to go to position in variable 'pos'   
        delay(1500);
        myservo.detach();  
        lookingLeft = 0;
      }
  }
}

