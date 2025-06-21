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

  Serial.begin(9600);
  Serial.println("Entering setup");
  Wire.begin();
  Wire.beginTransmission(0xEF);//this is the module address //after this we should have an ack from the bmp180
  //we are beginning with EF as this means we want to read the eeprom first
  Serial.println("here1");

  Wire.requestFrom(0xAA,1); //requesting one byte from the upper address first

  Serial.println("here2");
  AC1 = Wire.read();//reads in the requested data
  AC1 = AC1 << 8; //shift left by 8 bits since we read in MSB FIRST
  Serial.print("val befr concat:"); Serial.print(AC1);

  Wire.requestFrom(0xAB,1); //requesting one byte from lower address
  AC1 = AC1 | Wire.read();//reads in the requested data
  Serial.print("val aftr concat:"); Serial.print(AC1); //for debugging

  Wire.requestFrom(0xAC,1);
  AC2 = AC2 << 8; //shift left by 8 bits since we read in MSB FIRST
  AC2 = Wire.read();

  Wire.requestFrom(0xAD,1);
  AC2 = AC2 | Wire.read();

  //
  Wire.requestFrom(0xAE,1);
  AC3 = Wire.read();
  AC3 = AC3 << 8; //shift left by 8 bits since we read in MSB FIRST


  Wire.requestFrom(0xAF,1);
  AC3 = AC3 | Wire.read();

  Wire.requestFrom(0xB0,1);
  AC4 = Wire.read();
  AC4 = AC4 << 8; //shift left by 8 bits since we read in MSB FIRST


  Wire.requestFrom(0xB1,1);
  AC4 = AC4 | Wire.read();

  Wire.requestFrom(0xB2,1);
  AC5 = Wire.read();
  AC5 = AC5 << 8; //shift left by 8 bits since we read in MSB FIRST


  Wire.requestFrom(0xB3,1);
  AC5 = AC5 | Wire.read();

  Wire.requestFrom(0xB4,1);
  AC6 = Wire.read();
  AC6 = AC6 << 8; //shift left by 8 bits since we read in MSB FIRST

  Wire.requestFrom(0xB5,1);
  AC6 = AC6 | Wire.read();

  Wire.requestFrom(0xB6,1);
  BB1 = Wire.read();
  BB1 = BB1 << 8; //shift left by 8 bits since we read in MSB FIRST

  Wire.requestFrom(0xB7,1);
  BB1 = BB1 | Wire.read();

  Wire.requestFrom(0xB8,1);
  B2 = Wire.read();
  B2 = B2 << 8; //shift left by 8 bits since we read in MSB FIRST

  Wire.requestFrom(0xB9,1);
  B2 = B2 | Wire.read();

  Wire.requestFrom(0xBA,1); //lower case a in the docs may be a typo here?
  MB = Wire.read();
  MB = MB << 8; //shift left by 8 bits since we read in MSB FIRST

  Wire.requestFrom(0xBB,1);
  MB = MB | Wire.read();


  Wire.requestFrom(0xBC,1);
  MC = Wire.read();
  MC = MC << 8; //shift left by 8 bits since we read in MSB FIRST

  Wire.requestFrom(0xBD,1);
  MC = MC | Wire.read();

  Wire.requestFrom(0xBE,1);
  MD = Wire.read();
  MD = MD << 8; //shift left by 8 bits since we read in MSB FIRST

  Wire.requestFrom(0xBF,1);
  MD = MD | Wire.read();

  Wire.endTransmission();


}

void loop() {

  //reading temp repeatedly

  //we now have initalized everything, we move to the 2nd box on page 15 read uncompensated temperature value

  Wire.beginTransmission(0xEE); //start transmission in write mode

  Wire.write(0xF4);//SEND REGISTER ADDRESS
  Wire.write(0x2E);//write 2E into the address

  vTaskDelay(5);//WAIT 5ms as per documentation

  short MSB = 0;
  short LSB = 0;

  Wire.endTransmission(); //end transmission as we now want to read

  Wire.beginTransmission(0xEF);

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