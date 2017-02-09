//#include "DHT22_test/PietteTech_DHT.h"  // Uncomment if building in IDE
#include "PietteTech_DHT.h"  // Uncommend if building using CLI

#define DHTTYPE  DHT22       // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   0           // Digital pin for communications

//Constants
const size_t READ_BUF_SIZE = 64;
const unsigned long RESPONSE_TIMEOUT = 10000;

//declaration
int toggleRelay(String command);
int setTemperatureHoneywell(String command);
int sendToHoneywell(String command);
Timer readDHT22timer(5000, getDHT22values);


// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE);

//global variables
unsigned long lastTime = 0;

//Cloud Variable
double tempCloud = -1;
double humiCloud = -1;


void setup()
{
    Particle.variable("temperature", tempCloud);
    Particle.variable("humidity", humiCloud);
    Particle.function("toggleRelay", toggleRelay);
    Particle.function("setTempHoney", setTemperatureHoneywell);

    Serial1.begin(2400, SERIAL_8E1);
    Serial.begin(9600);

    //Comment the lower line if no DHT Sesnsor is used
    readDHT22timer.start();
}


void loop()
{

}

void getDHT22values (){

  //TODO: catch DHT errors and stop timer? or ignore all error?

  int result = DHT.acquireAndWait();

  tempCloud = (double) DHT.getCelsius();
  humiCloud = (double) DHT.getHumidity();


}


int setTemperatureHoneywell(String command)
{

  int targetTemp = command.toInt();                               //Unit is 1/10° C
  int targetTempOffset = targetTemp - 60;                         //Offset is 60 (=6° C), Unit is 1/10° C
  String writeDisplayRAM = "W13600";                              //0x136 is the memory location of the target Temperature on the Display
  String writeMotorRam = "W20c10";                                //0x20c is the memory location of the target temperatur for the DC Motor

  if (targetTempOffset < 16) {                                    //add a "0" if target Temperature is only 1 Hex character
    writeDisplayRAM += "0";
    writeMotorRam += "0";
  }
  writeDisplayRAM += String(targetTempOffset, HEX);               //transform the target Temperature from DEC to HEX
  writeMotorRam += String(targetTempOffset, HEX);



  //send empty commands till "Honeywell" is responsing.
  while (!Serial1.available()){
		Serial1.println("K");
	  delay(50);
	}

  while (Serial1.available()) {
    Serial1.read();																		           	//delete response from empty commands
  }


  //send both commands to Honeywell
  int DisplayResponse = sendToHoneywell(writeDisplayRAM);
  delay(20);                                                    //delay for Honeywell to processing the command
  int MotorResponse = sendToHoneywell(writeMotorRam);


  //check if execution was successfull
  if (DisplayResponse == MotorResponse){
    switch (DisplayResponse){
      case 1:
        return 1;                 //execution was successfull for both commands
        break;
      case -1:
        return -1;                //Error1: Response different as expected
        break;
      case -2:
        return -2;                //Error2: readBuf overflow
        break;
      case -3:
        return -3;                //Error3: Response Timeout
        break;
      default:
        return -1;                //Error1: Response different as expected
        break;
    }
  }
  else{return -1;}                //Error1: Response different as expected


}


int sendToHoneywell(String command)
{
  char readBuf[READ_BUF_SIZE];
  size_t readBufOffset = 0;
  bool responseComplete = false;

  //send command to Honeywell
  Serial1.printlnf(command);



  lastTime = millis();
  while ((millis() - lastTime < RESPONSE_TIMEOUT) && !responseComplete){

    //receive the Honeywell response
    while (Serial1.available()) {
  		if (readBufOffset < READ_BUF_SIZE) {
  			char c = Serial1.read();
  			readBuf[readBufOffset++] = c;
        if (c == '\n'){responseComplete = true;}

  		}
  		else {
        return -2;                                        //Error2: readBuf overflow
  			readBufOffset = 0;
  		}
  	}
  }

  if ((millis() - lastTime > RESPONSE_TIMEOUT) && !responseComplete){
    return -3;                                            //Error3: Response Timeout
  }


  String expectedResponse = command;
  expectedResponse.setCharAt(0, 'M');
  expectedResponse.toUpperCase();                         //the HEX convert the letter in lower case, but Honeywell response with upper case
  expectedResponse += String("\r\n");

  if(expectedResponse == "M20C100F\r\n")expectedResponse.setCharAt(7, '0');               //change character 7 from 'F' to '0' only for motor command if honeywell is set to OFF

  //Serial.printlnf("Received from Honeywell: %s", readBuf);
  //Serial.printlnf(expectedResponse);


  //check if response is equal to the expectedResponse
  String readBufString = readBuf;
  if (readBufString == expectedResponse){											//check if setting targetTemp was successfull
    return 1;
  }
  else {
    return -1;                                              //Error1: Response different as expected
  }

}

int toggleRelay(String command)
{
  bool value = 0;
  int toggleTime = 0;

  //convert ascii to integer
  int pinNumber = command.charAt(1) - '0';
  toggleTime = command.substring(8,15).toInt();




  //relay is switched on a Low Output. Without power or on High Output the relay is unswitched.
  //On startup the output is undefind => same Relay State as High Output (unswitched)
  //if the relay is switched on low or high output can be changed in the android app settings (Particle Remote)


  if(command.substring(3,7) == "HIGH") value = 1;
    else if(command.substring(3,6) == "LOW") value = 0;
    else return -2;

  if(command.startsWith("D"))
  {

      //Sanity check to see if the pin is used from the DHT sensor
      if (pinNumber == DHTPIN) return -3;
      //Sanity check to see if the pin numbers are within limits
      if (pinNumber < 0 || pinNumber > 7) return -1;

      pinMode(pinNumber, OUTPUT);
      digitalWrite(pinNumber, value);

      //start a timer interrupt then return 1 (before interrupt execution)
      if (toggleTime>0)
      {

        delay(toggleTime*1000);
        value =! value;
        digitalWrite(pinNumber, value);
      }

      return 1;


  }
  else if(command.startsWith("A"))
  {
      //Sanity check to see if the pin numbers are within limits
      if (pinNumber < 0 || pinNumber > 5) return -1;

      pinNumber = pinNumber+10;
      pinMode(pinNumber, OUTPUT);
      digitalWrite(pinNumber, value);

      if (toggleTime>0)
      {
        delay(toggleTime*1000);
        value =! value;
        digitalWrite(pinNumber, value);
      }

      return 1;
  }
}
