#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Canbus.h>
#include <mcp_can.h>
#include <TinyGPS++.h>
#include <LiquidCrystal_I2C.h>


// Define Joystick connection 
#define UP     A1
#define RIGHT  A2
#define DOWN   A3
#define CLICK  A4 
#define LEFT   A5

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Device address for ADXL345 accelerometer as specified in data sheet 
#define DEVICE (0x53) 
byte accel_buf[6];
char POWER_CTL = 0x2D;	//Power Control Register
char DATA_FORMAT = 0x31;
char DATAX0 = 0x32;	//X-Axis Data 0
char DATAX1 = 0x33;	//X-Axis Data 1
char DATAY0 = 0x34;	//Y-Axis Data 0
char DATAY1 = 0x35;	//Y-Axis Data 1
char DATAZ0 = 0x36;	//Z-Axis Data 0
char DATAZ1 = 0x37;	//Z-Axis Data 1

// CSV log file definitions
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
      digitalWrite(LED3, HIGH);
      logFileNum++;
    } else {
      digitalWrite(LED3, LOW); 
    }
  }
  last_interrupt_time = interrupt_time;
}

MCP_CAN CAN0(CAN_CS);

void setup() {
  attachInterrupt(1, logger_ISR, FALLING); //interrupt on pin 3 of Mega2560

  lcd.init();
  lcd.backlight();
  lcd.print("Logger ON");

  Serial.begin(115200);
  Serial1.begin(GPSBaud);

  pinMode(LED3, OUTPUT);
  digitalWrite(LED3, LOW);

  // joystick closes to ground
  pinMode(UP,INPUT_PULLUP);
  pinMode(DOWN,INPUT_PULLUP);
  pinMode(LEFT,INPUT_PULLUP);
  pinMode(RIGHT,INPUT_PULLUP);
  pinMode(CLICK,INPUT_PULLUP);
  pinMode(sdcard_CS, OUTPUT);

  //Start i2c bus 
  Serial.print("Initializing Accelerometer...");
  Wire.begin();
  //Put the ADXL345 into Measurement Mode by writing 0x08 to the POWER_CTL register.
  writeI2C(POWER_CTL, 0x08);
  Serial.println("OK");

  Serial.print("Initializing CAN controller...");
  CAN0.begin(CAN_500KBPS);
  pinMode(can_INT, INPUT);
  Serial.println("OK");

  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(sdcard_CS)) {
    Serial.println("Card failed, or not present");
  } else {
    Serial.println("OK");
  }
}

void loop() {
  // Read incoming GPS data
  while (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
    }
  }
    
  String dataString = "";
  dataString += String(millis());
  dataString += DELIMIT_CHAR;
  char charVal[20];               //temporarily holds data from vals 
  String latString, lonString, spdString;     //GPS data

  latString = gps.location.isValid() ? dtostrf(gps.location.lat(), 4, 6, charVal) : "0.0";  //4 is mininum width, 6 is precision
  dataString += latString;
  dataString += DELIMIT_CHAR;
  lonString = gps.location.isValid() ? dtostrf(gps.location.lng(), 4, 6, charVal) : "0.0";  //4 is mininum width, 6 is precision
  dataString += lonString;
  dataString += DELIMIT_CHAR;
  spdString = gps.speed.isValid() ? dtostrf(gps.speed.kmph(), 4, 2, charVal) : "0.0";  //4 is mininum width, 6 is precision
  dataString += spdString;
  dataString += DELIMIT_CHAR;

  bool hasSpeed = false;
  bool hasRPM = false;
  bool hasAccel = false;
  bool hasSteering = false;
  bool hasBrake = false;
  char speed_buf [10];
  char rpm_buf [10];
  char accel[10];
  char steering[10];
  char brake[10];
  unsigned int speedKPH;
  unsigned int rpm;
  int steeringAngle;
  float x, y, z;

  while (!hasRPM || !hasAccel || !hasSteering || !hasBrake) {
    if(!digitalRead(can_INT)) {
      CAN0.readMsgBuf(&len, rxBuf);
      rxId = CAN0.getCanId();
      //Serial.println(String(rxId));
      switch (rxId) {
//          case 0x370: // Speed in Kilometers Per Hour (KPH)
//            speedKPH = rxBuf[5];
//            sprintf(speed_buf, "%d, ", speedKPH);
//            hasSpeed = true;
//            break;
        case 0x360: // RPM
          rpm = ((unsigned int)rxBuf[1] << 8) | rxBuf[0];
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

  // Read all 6 accelerometer registers
  readI2C(DATAX0, 6, accel_buf);
  // Convert 2-bytes into a 10-bit signed value
  x = ((((int)accel_buf[1]) << 8) | accel_buf[0]) / 255.0 ;   
  y = ((((int)accel_buf[3]) << 8) | accel_buf[2]) / 255.0;
  z = ((((int)accel_buf[5]) << 8) | accel_buf[4]) / 255.0;

 // dataString += String(speed_buf);
  dataString += String(rpm_buf);
  dataString += String(brake);
  dataString += String(accel);
  dataString += String(steering);
  dataString += String(x);
  dataString += DELIMIT_CHAR;
  dataString += String(y);
  dataString += DELIMIT_CHAR;
  dataString += String(z);

  // Clear current screen 
  Serial.write(27);
  Serial.print("[2J");
  Serial.write(27);
  Serial.print("[H");

  Serial.println(dataString);
   
  if (isLogging) {
    sprintf(logFile, "lf%d.csv", logFileNum);
    File dataFile = SD.open(logFile, FILE_WRITE);

    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
    } else {
      Serial.println("error opening log file");
    }
  }
}

void writeI2C(byte address, byte val) {
  Wire.beginTransmission(DEVICE);  
  Wire.write(address);            
  Wire.write(val);               
  Wire.endTransmission();         
}

void readI2C(byte address, int num, byte accel_buf[]) {
  int i = 0;

  Wire.beginTransmission(DEVICE);
  Wire.write(address);          
  Wire.endTransmission();      

  Wire.beginTransmission(DEVICE);
  Wire.requestFrom(DEVICE, num);

  while(Wire.available()) {    
    accel_buf[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();     
}
