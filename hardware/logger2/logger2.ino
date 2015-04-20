#include <SD.h>
#include <SPI.h>
#include <mcp_can.h>
#include <TinyGPS++.h>


/* Define Joystick connection */
#define UP     A1
#define RIGHT  A2
#define DOWN   A3
#define CLICK  2 
#define LEFT   A5

#define MAX_FILENAME_LEN 100 //bytes
#define NAN_STRING "NaN, "
#define DELIMIT_CHAR ", "

const int CAN_CS = 49;
const int sdcard_CS = 53;
const int accel_CS = 45;
const int can_INT = 48; // INT pin of mcp2515

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

static TinyGPSPlus gps;
static const uint32_t GPSBaud = 115200;

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

MCP_CAN CAN0(CAN_CS);

void setup() {
  attachInterrupt(0, logger_ISR, FALLING);

  Serial.begin(115200);
  Serial1.begin(GPSBaud);

  pinMode(LED3, OUTPUT);

  // joystick closes to ground
  pinMode(UP,INPUT_PULLUP);
  pinMode(DOWN,INPUT_PULLUP);
  pinMode(LEFT,INPUT_PULLUP);
  pinMode(RIGHT,INPUT_PULLUP);
  pinMode(CLICK,INPUT_PULLUP);
  pinMode(sdcard_CS, OUTPUT);

  Serial.print("Initializing CAN controller...");
  CAN0.begin(CAN_500KBPS);
  pinMode(can_INT, INPUT);

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(sdcard_CS)) {
    Serial.println("Card failed, or not present");
  } else {
    Serial.println("OK");
  }
}

void loop() {
  while (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
    }
  }
  
  if (true) {
    digitalWrite(LED3, HIGH);

    String dataString = "";
    dataString += String(millis());
    dataString += DELIMIT_CHAR;
    char charVal[20];               //temporarily holds data from vals 
    String stringVal;     //data on buff is copied to this string

    stringVal = dtostrf(gps.location.lat(), 4, 6, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
    dataString += stringVal;
    dataString += DELIMIT_CHAR;
    stringVal = dtostrf(gps.location.lng(), 4, 6, charVal);  //4 is mininum width, 3 is precision; float value is copied onto buff
    dataString += stringVal;
    dataString += DELIMIT_CHAR;

    bool hasSpeed = false;
    bool hasRPM = false;
    bool hasAccel = false;
    char accel[10];
    char steering[10];
    char brake[10];
    char rpm_buf [10];
    char speed_buf [10];
    bool hasSteering = false;
    bool hasBrake = false;
    int steeringAngle;
    unsigned int rpm;
    unsigned int speed;

    while (!hasSpeed || !hasRPM || !hasAccel || !hasSteering || !hasBrake) {
      if(!digitalRead(can_INT)) {
        CAN0.readMsgBuf(&len, rxBuf);
        rxId = CAN0.getCanId();
        switch (rxId) {
          case 0x172: // Speed
            speed = rxBuf[5];
            sprintf(speed_buf, "%d, ", speed);
            hasSpeed = true;
            break;
          case 0x360: // RPM
            rpm = rxBuf[1];
            rpm = (rpm << 8) | rxBuf[0];
            rpm *= 4;
            sprintf(rpm_buf, "%d, ", rpm);
            hasRPM = true;
            break;
          case 0xd1: // Brake Pressure
            sprintf(brake, "%d, ", rxBuf[2]);
            hasBrake = true;
            break;
          case 0x140: // Accelerator Pedal
            sprintf(accel, "%d, ", rxBuf[0]);
            hasAccel = true;
            break;
          case 0xd0: // Steering Angle
            steeringAngle = rxBuf[1];
            steeringAngle = (steeringAngle << 8) | rxBuf[0];
            sprintf(steering, "%d, ", steeringAngle);
            hasSteering = true;
            break;
        }
      }
    }

    dataString += String(speed);
    dataString += String(rpm_buf);
    dataString += String(brake);
    dataString += String(accel);
    dataString += String(steering);

    // Clear current screen 
    Serial.write(27);
    Serial.print("[2J");
    Serial.write(27);
    Serial.print("[H");

    Serial.println(dataString);
    sprintf(logFile, "lf%d.csv", logFileNum);
    File dataFile = SD.open(logFile, FILE_WRITE);

    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    } else {
      Serial.println("error opening datalog.txt");
    }

    digitalWrite(LED3, LOW); 
  }
}
