#include <SD.h>
#include <Canbus.h>
#include <SPI.h>


/* Define Joystick connection */
#define UP     A1
#define RIGHT  A2
#define DOWN   A3
#define CLICK  2 
#define LEFT   A5

#define MAX_FILENAME_LEN 100 //bytes

const int sdcard_CS = 53;
const int can_INT = 48; // INT pin of mcp2515



char canBuffer[512];  //Data will be temporarily stored to this buffer before being written to the file
char lat_str[14];
char lon_str[14];

int LED2 = 47;
int LED3 = 46;

volatile int isLogging = LOW;
volatile int logFileNum = 0;
char logFile [MAX_FILENAME_LEN];

void logger_ISR(void) {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
    isLogging = !isLogging;
    if (isLogging) {
      logFileNum++;
    }
  }
  last_interrupt_time = interrupt_time;
}


void setup() {
  attachInterrupt(0, logger_ISR, FALLING);

  Serial.begin(115200);

  pinMode(LED3, OUTPUT);

  // joystick closes to ground
  pinMode(UP,INPUT_PULLUP);
  pinMode(DOWN,INPUT_PULLUP);
  pinMode(LEFT,INPUT_PULLUP);
  pinMode(RIGHT,INPUT_PULLUP);
  pinMode(CLICK,INPUT_PULLUP);
  pinMode(sdcard_CS, OUTPUT);

  Serial.print("Initializing CAN controller...");
  if(!Canbus.init(CANSPEED_500)) { 
    Serial.println("FAILED");
  } else {
    Serial.println("OK");
  } 

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(sdcard_CS)) {
    Serial.println("Card failed, or not present");
  } else {
    Serial.println("OK");
  }

  delay(1000); 
}

void loop() {
  if (isLogging) {
    String dataString = "";
    dataString += String(millis());
    dataString += ',';
    Serial.println("here");

    digitalWrite(LED3, HIGH);

    if(Canbus.ecu_req(ENGINE_RPM,canBuffer) == 1) {
      Serial.print("RPM: ");
      Serial.println(canBuffer);  
      dataString += String(canBuffer);
      dataString += ',';
    } 

    if(Canbus.ecu_req(VEHICLE_SPEED,canBuffer) == 1) {
      Serial.print("Speed:  ");
      Serial.println(canBuffer); 
      dataString += String(canBuffer);
      dataString += ',';
    }

    if(Canbus.ecu_req(ENGINE_COOLANT_TEMP,canBuffer) == 1) {
      Serial.print("Coolant Temp:  ");
      Serial.println(canBuffer);  
      dataString += String(canBuffer);
      dataString += ',';
    }

    if(Canbus.ecu_req(THROTTLE,canBuffer) == 1) {
      Serial.print("Throttle: ");
      Serial.println(canBuffer);  
      dataString += String(canBuffer);
    } 

    digitalWrite(LED3, LOW); 

    sprintf(logFile, "lf%d.csv", logFileNum);
    File dataFile = SD.open(logFile, FILE_WRITE);

    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    } else {
      Serial.println("error opening datalog.txt");
    }
  }
}
