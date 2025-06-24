#include <Arduino.h>
#include <Wire.h> //to help abstract away library code
#include <cmath> //for pow

//wifi includes
#include <WiFi.h>
#include <HTTPClient.h>

//file reading includes
#include <fstream>
#include <string>


/*Led's

*/

const short GREENLED = 23;
const short REDLED = 19;
const short BLUELED = 18;


/*Wifi global vars*/


const char * ssid = "";
const char * password = "";
const char * serverName = "";


/*End of Wifi stuff*/

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
long prevT = 0;
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




  Wire.endTransmission();





}


void setup() {
  //from page 15 of the manual
  //Step 1, read calibration data from eeprom, 16 bits, MSB first

  /*
  77 address is for writing
  77 addr is for reading
  -There may be a problem with 7 or 8 bit addressing with arduino wire library
  */

  /*Setup LED's*/

  pinMode(REDLED,OUTPUT);
  pinMode(BLUELED,OUTPUT);
  pinMode(GREENLED,OUTPUT);

  //

  Serial.begin(115200);
  Serial.println("Entering setup");
  Wire.begin(); //this should send the start condition
  



  readReg(0xAA,&AC1);
  readReg(0xAC,&AC2);
  readReg(0xAE,&AC3);
  readRegu(0xB0,&AC4);//these need to be unsigned as per datasheet
  readRegu(0xB2,&AC5);
  readRegu(0xB4,&AC6);//

  readReg(0XB6,&BB1); //BB1 AS B1 IS RESERVED FOR OTHER FUNCTIONALITY
  readReg(0xB8,&B2);
  readReg(0xBA,&MB);
  readReg(0xBC,&MC);
  readReg(0xBE,&MD);
  
  
  //setup wifi

  Serial.println(ssid);
  Serial.println(password);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);


  WiFi.begin(ssid, password); //need to cast the strings to char pointers

  Serial.println("Connecting");
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected, ip: ");
  Serial.print(WiFi.localIP());

  //turn on blue led once connected to wifi
  digitalWrite(BLUELED,HIGH);



}

void loop() {

  //reading temp repeatedly
  
  //we now have initalized everything, we move to the 2nd box on page 15 read uncompensated temperature value


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



  UT = (MSB << 8) + LSB; 


  //calculate true temperature

  long X1 = (UT - AC6) * (AC5/(pow(2,15)));

  long X2 = MC * ((pow(2,11))/(X1+MD));

  long B5 = X1 + X2;

  T = (B5 + 8)/pow(2,4);// this should be the final temperature IN 0.1 DEG CEL

  T = T/10; //divide by 10 so we are in deg C

  //print variables for debugging



  if(prevT != T){
    
  Serial.print("Temperature is: ");
  Serial.print(T);
  Serial.print(" degrees Celcius");
  Serial.println();//make a new line after enverything
  prevT = T;


  //we now want to send this temp to the web server
  if(WiFi.status() == WL_CONNECTED){//if we are connected to internet
    HTTPClient http; //create new http client
    WiFiClient client;
    
    http.begin(client,serverName);
  
    //headers
    http.addHeader("Content-Type","application/json; charset = utf-8");

    //data
    std::string data = "{\"temp\" : \"" +  std::to_string(T) + "\"}" ;
    
    int httpResponseCode = http.POST(data.c_str()); //possible source of error if c_str does not do what I think it does

    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    if(httpResponseCode == 200){//flash green led when we have a successful POST request.
      digitalWrite(GREENLED,HIGH);
      delay(200);
      digitalWrite(GREENLED,LOW);
    }else{
      digitalWrite(REDLED,HIGH);
    }
  }

  //
  vTaskDelay(10000);//delay for 10 sec

  digitalWrite(REDLED,LOW);//reset ledled if we had an error.

  }


}

/*
Current power consumption:
240mw


*/