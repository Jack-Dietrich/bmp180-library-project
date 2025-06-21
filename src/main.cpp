#include <Arduino.h>
#include <Wire.h> //to help abstract away library code
#include <cmath> //for pow


/*Variable definitions*/
short AC1 = 0;
short AC2 = 0;
short AC3 = 0;
unsigned short AC4 = 0;
unsigned short AC5 = 0;
unsigned short AC6 = 0;

short BB1 = 0; //had to change this from B1 as it was defined in wire.h
short B2 = 0;
short MB = 0;
short MC = 0;
short MD = 0;

long UT = 0;
long T = 0; //this is the final temp
/*End of variable definitions*/

void setup() {
  //from page 15 of the manual
  //Step 1, read calibration data from eeprom, 16 bits, MSB first

  /*
  77 address is for writing
  77 addr is for reading
  -There may be a problem with 7 or 8 bit addressing with arduino wire library
  */

  Serial.begin(9600);
  Serial.println("Entering setup");
  Wire.begin(); //this should send the start condition
  
  
  Wire.beginTransmission(0x77);//this is the module address //after this we should have an ack from the bmp180

  Wire.write(0xAA);//write the reg we want to read from //this just queues them

  //should get an asks msg here

  Wire.endTransmission(); //this actually writes them

  Wire.beginTransmission(0x77); //now reading

  AC1 = Wire.read(); //read msb,
  AC1 = AC1 << 8;
  AC1 = AC1 | Wire.read(); //read lsb

  Wire.endTransmission();

  // Read AC2
  Wire.beginTransmission(0x77);
  Wire.write(0xAC);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  AC2 = Wire.read();
  AC2 = AC2 << 8;
  AC2 = AC2 | Wire.read();
  Wire.endTransmission();

  // Read AC3
  Wire.beginTransmission(0x77);
  Wire.write(0xAE);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  AC3 = Wire.read();
  AC3 = AC3 << 8;
  AC3 = AC3 | Wire.read();
  Wire.endTransmission();

  // Read AC4
  Wire.beginTransmission(0x77);
  Wire.write(0xB0);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  AC4 = Wire.read();
  AC4 = AC4 << 8;
  AC4 = AC4 | Wire.read();
  Wire.endTransmission();

  // Read AC5
  Wire.beginTransmission(0x77);
  Wire.write(0xB2);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  AC5 = Wire.read();
  AC5 = AC5 << 8;
  AC5 = AC5 | Wire.read();
  Wire.endTransmission();

  // Read AC6
  Wire.beginTransmission(0x77);
  Wire.write(0xB4);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  AC6 = Wire.read();
  AC6 = AC6 << 8;
  AC6 = AC6 | Wire.read();
  Wire.endTransmission();

  // Read BB1
  Wire.beginTransmission(0x77);
  Wire.write(0xB6);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  BB1 = Wire.read();
  BB1 = BB1 << 8;
  BB1 = BB1 | Wire.read();
  Wire.endTransmission();

  // Read B2
  Wire.beginTransmission(0x77);
  Wire.write(0xB8);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  B2 = Wire.read();
  B2 = B2 << 8;
  B2 = B2 | Wire.read();
  Wire.endTransmission();

  // Read MB
  Wire.beginTransmission(0x77);
  Wire.write(0xBA);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  MB = Wire.read();
  MB = MB << 8;
  MB = MB | Wire.read();
  Wire.endTransmission();

  // Read MC
  Wire.beginTransmission(0x77);
  Wire.write(0xBC);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  MC = Wire.read();
  MC = MC << 8;
  MC = MC | Wire.read();
  Wire.endTransmission();

  // Read MD
  Wire.beginTransmission(0x77);
  Wire.write(0xBE);
  Wire.endTransmission();
  Wire.beginTransmission(0x77);
  MD = Wire.read();
  MD = MD << 8;
  MD = MD | Wire.read();
  Wire.endTransmission();




}

void loop() {

  //reading temp repeatedly
  
  //we now have initalized everything, we move to the 2nd box on page 15 read uncompensated temperature value

  Serial.println("In the loop");

  Wire.beginTransmission(0x77); //start transmission in write mode

  Wire.write(0xF4);//SEND REGISTER ADDRESS
  Wire.write(0x2E);//write 2E into the address

  vTaskDelay(5);//WAIT 5ms as per documentation

  short MSB = 0;
  short LSB = 0;

  Wire.endTransmission(); //end transmission as we now want to read

  Wire.beginTransmission(0x77);

  Wire.requestFrom(0xF6,1);
  MSB = Wire.read();

  Wire.requestFrom(0xF7,1);
  LSB = Wire.read();

  UT = MSB << 8 + LSB; //unsure about correct presidence here, shift should have higher presidence but that may not be intended


  //calculate true temperature

  long X1 = (UT - AC6) * AC5/(pow(2,15));
  long X2 = MC * (pow(2,11))/(X1+MD);
  long B5 = X1 + X2;
  T = (B5 + 8)/pow(2,4);// this should be the final temperature IN 0.1 DEG CEL
  T = T/10; //divide by 10 so we are in deg C


  Serial.print("Temperature is: " + T);
  Serial.print(" degrees Celcius");
  Serial.println();//make a new line after enverything




}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

/*

TODO:
-add a left bitwise shift as we are currently reading in the first byte than oring it which is not correct

0000 0000 0000 0000

read

0000 0000 1234 5678

//first value has been read, we now should shift it 2 bytes

1234 5678 0000 0000

then or in second value



*/