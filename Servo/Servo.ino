/* Sweep
  by BARRAGAN <http://barraganstudio.com>
  This example code is in the public domain.

  modified 8 Nov 2013
  by Scott Fitzgerald
  https://www.arduino.cc/en/Tutorial/LibraryExamples/Sweep
*/

#include <Servo.h>

Servo myservoIn;
Servo myservoOut;

int pos = 0;    // variable to store the servo position

void setup() {
  Serial.begin(112500);
  myservoIn.attach(5);  // attaches the servo on pin 9 to the servo object
  myservoOut.attach(4);
}

void loop() {
  myservoIn.write(90);
  myservoOut.write(90);
  for (pos = 0; pos <= 360; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    Serial.println(pos);
    myservoIn.write(pos);              // tell servo to go to position in variable 'pos'
    myservoOut.write(pos);
    delay(100);                       // waits 15 ms for the servo to reach the position
  }
  for (pos = 360; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    Serial.println(pos);
    myservoIn.write(pos);              // tell servo to go to position in variable 'pos'
    myservoOut.write(pos);
    delay(100);                       // waits 15 ms for the servo to reach the position
  }
}
