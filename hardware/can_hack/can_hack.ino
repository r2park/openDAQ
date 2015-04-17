#include <SD.h>
#include <SPI.h>
#include <Canbus.h>

#define MAX_FILENAME_LEN 100 //bytes
#define START_PID 200
#define MAX_PID 56

#define UP     A1
#define RIGHT  A2
#define DOWN   A3
#define CLICK  2 
#define LEFT   A5
const int sdcard_CS = 53;
const int can_INT = 48; // INT pin of mcp2515
char canBuffer[255];  //Data will be temporarily stored to this buffer before being written to the file

int LED2 = 47;
int LED3 = 46;
int i;
int count = 0;
void setup() {
  Serial.begin(115200);
  pinMode(UP,INPUT_PULLUP);
  pinMode(DOWN,INPUT_PULLUP);
  pinMode(LEFT,INPUT_PULLUP);
  pinMode(RIGHT,INPUT_PULLUP);
  pinMode(CLICK,INPUT_PULLUP);
  pinMode(sdcard_CS, OUTPUT);

  pinMode(LED3, OUTPUT);

  Serial.print("Initializing CAN controller...");
  if(!Canbus.init(CANSPEED_500)) { 
    Serial.println("FAILED");
  } else {
    Serial.println("OK");
  } 
}

void loop() {
    count++;
    digitalWrite(LED3, HIGH);
    String output = "";

    Canbus.ecu_scan(0,canBuffer);
    /*for (i=START_PID; i<MAX_PID + START_PID; i++) {*/
      /*Canbus.ecu_scan(i,canBuffer);*/
      /*output += String(canBuffer);*/
    /*}*/

    Serial.write(27);
    Serial.print("[2J");
    Serial.write(27);
    Serial.print("[H");
    /*Serial.print(output);*/
    Serial.println(String(canBuffer));
        
    digitalWrite(LED3, LOW); 
  
}
