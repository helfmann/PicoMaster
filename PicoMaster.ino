#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#define W_CLK 8       // Pin 8 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 9       // Pin 9 - connect to freq update pin (FQ)
#define DATA 10       // Pin 10 - connect to serial data load pin (DATA)
#define RESET 11      // Pin 11 - connect to reset pin (RST).

#define pulseHigh(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW); }


String incomingByte; //defines incoming sring
String MSG_type; //Start of input message, first character
String CND_type_0, CND_type_1; //Condition Message type, str or end
String input;
long unsigned int freq = 1000;
long unsigned int FreqOld = freq;
String Start_Freq;
String End_Freq;
long unsigned int Start;
long unsigned int End;
int Sweep;
int PortOneTX = 4;
int PortOneRX = 5;

const String USB = "USB";
const String GRBL = "GRBL";

char endOfLine = '\n';

SoftwareSerial portOne(PortOneRX, PortOneTX);

String grblBytes = "";
String USBBytes = "";
String stringtoWrite = "";

void tfr_byte(byte data)
{
  for (int i = 0; i < 8; i++, data >>= 1) {
    digitalWrite(DATA, data & 0x01);
    pulseHigh(W_CLK);   //after each bit sent, CLK is pulsed high
  }
}

void sendFrequency(double frequency) {
  int32_t freq = frequency * 4294967295 / 125000000; // note 125 MHz clock on 9850
  for (int b = 0; b < 4; b++, freq >>= 8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);   // Final control byte, all 0 for 9850 chip
  pulseHigh(FQ_UD);  // Done!  Should see output
}

void setup() {
  // put your setup code here, to run once:;
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);

  pulseHigh(RESET);
  pulseHigh(W_CLK);
  pulseHigh(FQ_UD);

  Serial.begin(9600);
  Serial.println("Hello there!");
  portOne.begin(9600);
  portOne.write("\r\n");
  delay(2);       ;
}

void loop() {
  if (portOne.available() > 0)//while (portOne.available())
  {
    stringtoWrite = portOne.readStringUntil(endOfLine);
    for (int i = 0; i < stringtoWrite.length(); i++)
    {
      Serial.write(stringtoWrite[i]);
    }
    Serial.write(endOfLine);

  }

  if (Serial.available() > 0) {


    incomingByte = Serial.readString();
    input = incomingByte;
    MSG_type = incomingByte.substring(0, 2);
    String    format1 = input.substring(6, 8);
    String    format2 = input.substring(12, 14);
    String   format3 = input.substring(18, 20);
    if (MSG_type == "sS" || MSG_type == "fS") {
      CND_type_1 = incomingByte.substring(5, 6);
      Start_Freq = incomingByte.substring(2, 5);
      End_Freq = incomingByte.substring(6, 9);
    }

    if (MSG_type == "sS" &&  CND_type_1 == "E" ) { // figure out if the setting is swing or fixed
      Start = Start_Freq.toInt();
      End = End_Freq.toInt();
      Start *= 1000;
      End *= 1000;
      freq = Start;
      Sweep = 1;
    }
    else if (MSG_type == "fS") {
      Start = Start_Freq.toInt();
      Start *= 1000;
      freq = Start;
      sendFrequency(freq);
      Sweep = 0;
    }

    if (incomingByte == "fstop") {
      sendFrequency(0);
      Sweep = 0;
    }
    stringtoWrite = Serial.readStringUntil(endOfLine);
    for (int i = 0; i < stringtoWrite.length(); i++)
    {
      portOne.write(stringtoWrite[i]);
    }
    portOne.write(endOfLine);

  }

  if (Sweep = 1) {
    for (freq = Start; freq <= End; freq += 100) { //this is for sweep frequency
      sendFrequency(freq);  // freq
      delay(10);
    }
    freq = Start;
  }

}
