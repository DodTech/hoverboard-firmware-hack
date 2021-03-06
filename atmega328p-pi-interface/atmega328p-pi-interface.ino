//  ROV MCU 1/29/2019 //

#include <SoftwareSerial.h>

SoftwareSerial pie(2, 3); //Rx, Tx

int speedl = 0;
int speedr = 0;
int timeDelay = 20; //Loop delay in ms
int velocityMax = 100; //Max wheel velocity in cm/s
int velocityMin = 30;
int lastL;
int lastR;

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = {0};
int RightDistanceInput = 0;
int LeftDistanceInput = 0;
int AccelerationInput = 0;

boolean newData = false;

//============

void setup() {
  Serial.begin(19200);
  pie.begin(9600);
  pie.println("This program expects a message (Control mode) and three integers: Left Distance, Right Distance, Acceleration.");
  pie.println("Enter data in this style <Hello, 27, 12, 24>  ");
  pie.println();
}

//============

void loop() {
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);

    parseData();
    showParsedData();
    newData = false;
  }
  SafetyCheck();


}

//============

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = pie.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//============

void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  RightDistanceInput = atoi(strtokIndx);     // convert this part to an integer

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  LeftDistanceInput = atoi(strtokIndx);     // convert this part to an integer

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  AccelerationInput = atoi(strtokIndx);     // convert this part to an integer

}

//============

void showParsedData() {
  pie.print("Message ");
  pie.println(messageFromPC);
  pie.print("RightDistanceInput: ");
  pie.println(RightDistanceInput);
  pie.print("LeftDistanceInput: ");
  pie.println(LeftDistanceInput);
  pie.print("AccelerationInput: ");
  pie.println(AccelerationInput);
}

void SafetyCheck() { // Experimental timeout check. This function may screw with control though.
  if ((LeftDistanceInput == 0) && (RightDistanceInput == 0)) {
    pie.println("No Data recieved. Device timeout active")
    speedl = 0;
    speedr = 0;
    Serial.write((uint8_t *) &speedl, sizeof(speedl));
    Serial.write((uint8_t *) &speedr, sizeof(speedr));
    delay(250);
  }
  else {
    Compute();
    pie.print("ROV Active. SpeedL: ");
    pie.print(lastL/10);
    pie.print("% SpeedR: ");
    pie.print(LastR/10);
    pie.println("%");
  }
}
