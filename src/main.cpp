#include <Arduino.h>
#include <Wire.h> //to help abstract away library code
#include <cmath> //for pow


/*Variable definitions*/
short AC1 = 408;
short AC2 = -72;
short AC3 = -14383;
unsigned short AC4 = 32741;
unsigned short AC5 = 32757;
unsigned short AC6 = 23153;

short BB1 = 6190; //had to change this from B1 as it was defined in wire.h
short B2 = 4;
short MB = -32768;
short MC = -8711;
short MD = 2868;

long UT = 0;
long T = 0; //this is the final temp
/*End of variable definitions*/

//@param reg should be supplied in hex
//@param var is a pointer to the variable that is to be read into
void readReg(int reg, short * var){ //unsure if I can pass this as a void pointer, I need both short and unsigned short values // ONLY USE FOR SHORT VALUES
  Wire.beginTransmission(0x77);//this is the module address only the first 7 bits

  Wire.write(reg);//we want to read from this register

  Wire.endTransmission(); //end transmission, but we don't want to send stop, just restart as per datasheet

  Wire.requestFrom(0x77,2); //we now request 2 bytes, from the bmp180, this should read from 0xAA and 0X AB respectfully (hopefully) // sends the stop (p) signal once we have read the register

  
  *var = (Wire.read() << 8) | Wire.read();



  //may need to end transmission each time?

  Wire.endTransmission();

  /*
  Changes
  -Added end transmission, removed true from requestfrom 
  -made the read more succinct, fixed multiple derefrences
  
  */
}

void readRegu(int reg, unsigned short * var){ //unsure if I can pass this as a void pointer, I need both short and unsigned short values // ONLY USE FOR SHORT VALUES

  Wire.beginTransmission(0x77);//this is the module address only the first 7 bits

  Wire.write(reg);//we want to read from this register

  Wire.endTransmission(); //end transmission, but we don't want to send stop, just restart as per datasheet

  Wire.requestFrom(0x77,2); //we now request 2 bytes, from the bmp180, this should read from 0xAA and 0X AB respectfully (hopefully) // sends the stop (p) signal once we have read the register

  
  *var = (Wire.read() << 8) | Wire.read();


  //may need to end transmission each time?

  Wire.endTransmission();


  //may need to end transmission each time?
}


void setup() {
  //from page 15 of the manual
  //Step 1, read calibration data from eeprom, 16 bits, MSB first

  /*
  77 address is for writing
  77 addr is for reading
  -There may be a problem with 7 or 8 bit addressing with arduino wire library
  */

  Serial.begin(115200);
  Serial.println("Entering setup");
  Wire.begin(); //this should send the start condition
  



  readReg(0xAA,&AC1);
  readReg(0xAC,&AC2);
  readReg(0xAE,&AC3);
  readRegu(0xB0,&AC4);//these need to be unsigned
  readRegu(0xB2,&AC5);
  readRegu(0xB4,&AC6);//

  readReg(0XB6,&BB1); //BB1 AS B1 IS RESERVED FOR OTHER FUNCTIONALITY
  readReg(0xB8,&B2);
  readReg(0xBA,&MB);
  readReg(0xBC,&MC);
  readReg(0xBE,&MD);
  
  




}

void loop() {

  //reading temp repeatedly
  
  //we now have initalized everything, we move to the 2nd box on page 15 read uncompensated temperature value

  Serial.println("In the loop");

  Wire.beginTransmission(0x77); //start transmission in write mode

  Wire.write(0xF4);//SEND REGISTER ADDRESS
  Wire.write(0x2E);//write 2E into the address

  Wire.endTransmission(); //end transmission as we now want to read // this sends the writes, then we wait

  delay(5);//WAIT 5ms as per documentation //replaced with just delay instead of vtaskdelay for debugging 

  short MSB = 0;
  short LSB = 0;



  Wire.beginTransmission(0x77);

  Wire.write(0xF6); // we want to read from f6 for MSB and F7 for lsb

  Wire.endTransmission(false);

  Wire.requestFrom(0x77,1);
  MSB = Wire.read();

  Wire.requestFrom(0x77,1);
  LSB = Wire.read();

  Serial.print("here");

  UT = (MSB << 8) + LSB; //unsure about correct presidence here, shift should have higher presidence but that may not be intended


  //calculate true temperature

  long X1 = (UT - AC6) * (AC5/(pow(2,15)));

  long X2 = MC * ((pow(2,11))/(X1+MD));

  long B5 = X1 + X2;

  T = (B5 + 8)/pow(2,4);// this should be the final temperature IN 0.1 DEG CEL

  T = T/10; //divide by 10 so we are in deg C

  //print variables for debugging


  Serial.print("AC1: "); Serial.print(AC1);
  Serial.print(" AC2: "); Serial.print(AC2);
  Serial.print(" AC3: "); Serial.print(AC3);
  Serial.print(" AC4: "); Serial.print(AC4);
  Serial.print(" AC5: "); Serial.print(AC5);
  Serial.print(" AC6: "); Serial.print(AC6);
  Serial.print(" BB1: "); Serial.print(BB1);
  Serial.print(" B2: "); Serial.print(B2);
  Serial.print(" MB: "); Serial.print(MB);
  Serial.print(" MC: "); Serial.print(MC);
  Serial.print(" MD: "); Serial.print(MD);
  Serial.print(" UT: "); Serial.print(UT);
  Serial.print(" T: "); Serial.println(T);




  Serial.print("Temperature is: ");
  Serial.print(T);
  Serial.print(" degrees Celcius");
  Serial.println();//make a new line after enverything

  delay(100); //delay by 100ms so we are not being overwhelmed




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