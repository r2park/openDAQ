#include <Wire.h>


static const int therm_addr1 = 0x5A;
static const int therm_addr2 = 0x5B;
static const int therm_addr3 = 0x5C;
static const char OBJ_TEMP = 0x07;
static const char SMB_CHANGE = 0x0E;
byte therm_buf[6];
float temp1, temp2, temp3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  readI2C(therm_addr1, OBJ_TEMP, 3, therm_buf);
  temp1 = rawIRtoCelcius(therm_buf);
  readI2C(therm_addr2, OBJ_TEMP, 3, therm_buf);
  temp2 = rawIRtoCelcius(therm_buf);
  readI2C(therm_addr3, OBJ_TEMP, 3, therm_buf);
  temp3 = rawIRtoCelcius(therm_buf);
  Serial.print(temp1);
  Serial.print(" , ");
  Serial.print(temp2);
  Serial.print(" , ");
  Serial.println(temp3);
  delay(250);
}

void writeI2C(int device, byte address, byte val) {
  Wire.beginTransmission(device);  
  Wire.write(address);            
  Wire.write(val);               
  Wire.endTransmission();         
}

void readI2C(int device, byte address, int num, byte buffer[]) {
  int i = 0;

  Wire.beginTransmission(device);
  Wire.write(address);          
  Wire.endTransmission(false);      

  Wire.beginTransmission(device);
  Wire.requestFrom(device, num);

  while(Wire.available()) {    
    buffer[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();     
}

float rawIRtoCelcius(byte buffer[]) {
  float tempCelcius;
  tempCelcius = (buffer[1] << 8) | (buffer[0]);
  tempCelcius = (tempCelcius * 0.02) - 0.01;
  tempCelcius -= 273.15;
  
  return tempCelcius;
}
