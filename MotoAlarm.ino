#include "SoftwareSerial.h"
#include "Wire.h"

SoftwareSerial mySerial(3, 2);

const int MPU_ADDR = 0x68;

int16_t initial_accelerometer_x, initial_accelerometer_y, initial_accelerometer_z;
int16_t accelerometer_x, accelerometer_y, accelerometer_z; 
int16_t gyro_x, gyro_y, gyro_z;
int16_t temperature;

char tmp_str[7];

char incomingByte; 
String inputString;

boolean active = false;

void setup()
{
  Serial.begin(9600);
  delay(1000);
  
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); 
  Wire.write(0); 
  Wire.endTransmission(true);
  
  mySerial.begin(9600);

  while(!mySerial.available()){
    mySerial.println("AT");
    delay(1000); 
    Serial.println("Łączenie...");
  }
    
  Serial.println("Połączono!");  
      
  setReceivingSms();
}

void loop()
{  
  if(active == true){
    detectOrientationMove(); 
  }
  
  receivingSms();
  
  delay(1000);
}

void updateSerial()
{
  delay(1000);
  
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());
  }
}

char* convert_int16_to_str(int16_t i) 
{
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

void sendSMS(String message)
{
  mySerial.println("AT+CMGF=1");
  updateSerial();
  mySerial.println("AT+CMGS=\"+48xxxxxxxxx\"\r");
  updateSerial();
  mySerial.print(message);
  updateSerial();
  mySerial.write(26);
  updateSerial();
}

void makeCall()
{
  mySerial.println("AT");
  updateSerial();
  mySerial.println("ATD+ +48xxxxxxxxx;");
  updateSerial();
  delay(20000); 
  mySerial.println("ATH");
  updateSerial();
  delay(20000); 
}

void receivingSms(){
  if(mySerial.available()){
    delay(100);
  
    while(mySerial.available()){
      incomingByte = mySerial.read();
      inputString += incomingByte;
    }
    
    delay(100);      
    
    inputString.toUpperCase();

    if (inputString.indexOf("ON") > -1){
      Serial.println("Uruchomiono alarm");
      setAlarmOrientation();
      active = true;
      sendSMS("Uruchomiono alarm");
    } 
      
    if (inputString.indexOf("OFF") > -1){
      Serial.println("Wylaczono alarm");
      active = false;
      sendSMS("Wylaczono alarm");
    }

    if (inputString.indexOf("STATUS") > -1){
      if(active == true){
        Serial.println("Alarm uruchomiony");
        
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom(MPU_ADDR, 7*2, true);
  
        accelerometer_x = Wire.read()<<8 | Wire.read();
        accelerometer_y = Wire.read()<<8 | Wire.read(); 

        String x = convert_int16_to_str(accelerometer_x);
        String y = convert_int16_to_str(accelerometer_y);
        
        sendSMS("Alarm uruchomiony \nPozycja X:" + x + "\nPozycja Y:" + y);
      }else{
        Serial.println("Alarm wylaczony");
        String asd = convert_int16_to_str(initial_accelerometer_x);
        sendSMS("Alarm wylaczony");
      }      
    }
 
    delay(100);
    inputString = "";
  }
}

void setReceivingSms(){
  mySerial.println("AT");
  updateSerial();
  mySerial.println("AT+CMGF=1");
  updateSerial();
  mySerial.println("AT+CNMI=1,2,0,0,0");
  updateSerial();
}

void setAlarmOrientation()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 7*2, true);
  
  accelerometer_x = Wire.read()<<8 | Wire.read();
  accelerometer_y = Wire.read()<<8 | Wire.read(); 
  accelerometer_z = Wire.read()<<8 | Wire.read(); 

  initial_accelerometer_x = accelerometer_x;
  initial_accelerometer_y = accelerometer_y;
}

void detectOrientationMove()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false); 
  Wire.requestFrom(MPU_ADDR, 7*2, true); 
  
  accelerometer_x = Wire.read()<<8 | Wire.read();
  accelerometer_y = Wire.read()<<8 | Wire.read(); 
  accelerometer_z = Wire.read()<<8 | Wire.read();
  
  temperature = Wire.read()<<8 | Wire.read(); 
  
  gyro_x = Wire.read()<<8 | Wire.read();
  gyro_y = Wire.read()<<8 | Wire.read(); 
  gyro_z = Wire.read()<<8 | Wire.read(); 
  
  int difference_x = initial_accelerometer_x - accelerometer_x;
  int difference_x2 = accelerometer_x - initial_accelerometer_x;

  int difference_y = initial_accelerometer_y - accelerometer_y;
  int difference_y2 = accelerometer_y - initial_accelerometer_y;

  if(difference_x > 1000 || difference_x2 > 1000 || difference_y > 1000 || difference_y2 > 1000){
    Serial.println("Wykryto przechylenie");
    makeCall();
  }
}

void debugPrintOrintation()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 7*2, true);
  
  accelerometer_x = Wire.read()<<8 | Wire.read();
  accelerometer_y = Wire.read()<<8 | Wire.read(); 
  accelerometer_z = Wire.read()<<8 | Wire.read();
  Serial.print("aX = "); Serial.print(convert_int16_to_str(accelerometer_x));
  Serial.print(" | aY = "); Serial.print(convert_int16_to_str(accelerometer_y));
  Serial.print(" | aZ = "); Serial.print(convert_int16_to_str(accelerometer_z));
  Serial.print(" | tmp = "); Serial.print(temperature/340.00+36.53);
  Serial.print(" | gX = "); Serial.print(convert_int16_to_str(gyro_x));
  Serial.print(" | gY = "); Serial.print(convert_int16_to_str(gyro_y));
  Serial.print(" | gZ = "); Serial.print(convert_int16_to_str(gyro_z));
  Serial.println();
}

void debugDetectOrientationMove()
{
  if (accelerometer_x < 1000 && accelerometer_y < -4000){
     Serial.println("DO PRZODU");
  }else if(accelerometer_x < 1000 && accelerometer_y > 4000){
     Serial.println("DO TYLU");
  }else if(accelerometer_x > 4000 && accelerometer_y < 1000){
     Serial.println("NA LEWO");
  }else if(accelerometer_x < -4000 && accelerometer_y < 1000){
     Serial.println("NA PRAWO");
  }else{
     Serial.println("ROWNO");
  }
}
