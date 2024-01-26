//EEPROM
#include <EEPROM.h>
int address = 0;
//UART
#include <SoftwareSerial.h>
const byte RX = 2;
const byte TX = 3;
SoftwareSerial mySerial = SoftwareSerial(RX, TX);
String statusFromESP = "";
bool stringComplete = false;

String UIDSend = "";

long last = 0;
long scanCard = 0;
long buzzer = 0;
long closeBarrier = 0;
//RFID
#include <SPI.h>
#include <MFRC522.h>
//LCD I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//Servo
#include <Servo.h>
Servo myservoIn;
Servo myservoOut;

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

int servoClose = 8;
int servoOpen = 90;

int sensorIn = 7;
int sensorOut = 8;
int valueSensorIn;
int valueSensorOut;
int tempValueSensorIn;
int tempValueSensorOut;
int statusInOut = 0;

int slotParking;
String checkInOut = "";


LiquidCrystal_I2C lcd(0x27, 20, 4);
void setup()
{
  slotParking = EEPROM.read(address);
  lcd.init();
  lcd.backlight();
  printLCD();
  Serial.begin(115200); //Serial.begin() thiết lập giao tiếp nối tiếp giữa bo Arduino và LCD để hiển thị
  mySerial.begin(115200);
  last = millis();
  scanCard = millis(); //Nhận "Thời gian" hiện tại (thực sự là số mili giây kể từ khi chương trình bắt đầu)
  closeBarrier = millis();
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(sensorIn, INPUT);
  pinMode(sensorOut, INPUT);
  myservoIn.attach(5);
  myservoOut.attach(4);
  myservoIn.write(servoClose);
  myservoOut.write(servoClose);
  tempValueSensorIn = digitalRead(sensorIn);
  tempValueSensorOut = digitalRead(sensorOut);
}
void loop()
{
  //Đóng rào
  if (millis() - closeBarrier == 5000){ //Kiểm tra xem khoảng thời gian đã trôi qua nếu đủ 5s thì đóng rào
    slotParking -= statusInOut;
    EEPROM.write(address, slotParking);
    if (statusInOut == 1) {
      myservoIn.write(servoClose);//Nếu vậy, thay đổi trạng thái của rào
    }
    else if (statusInOut == -1) {
      myservoOut.write(servoClose);
    }
    statusInOut = 0;
    printLCD();
  }
  //Quẹt thẻ
  if (millis() - scanCard >= 1000) {
    valueSensorOut = digitalRead(sensorOut);
    if (slotParking > 0 || valueSensorOut == 0) {
      ScanCard();
      scanCard = millis();//Quan trọng để tiết kiệm thời gian bắt đầu của trạng thái LED hiện tại
    }
  }
  //Tắt còi
  if (millis() - buzzer >= 200) {
    analogWrite(6, 0);
  }
  //Read status from ESP:
  read_ESP();
}

void CheckInOut() {

  valueSensorIn = digitalRead(sensorIn);
  valueSensorOut = digitalRead(sensorOut);
  if (valueSensorIn == 0) {
    statusInOut = 1;
  } else if (valueSensorOut == 0) {
    statusInOut = -1;
  }
  //  if (valueSensorIn != tempValueSensorIn || valueSensorOut != tempValueSensorOut) {
  //    if (valueSensorIn == 0 && valueSensorOut == 1) {
  //      checkInOut += "02";
  //    }
  //    //    else if (valueSensorIn == 0 && valueSensorOut == 0) {
  //    //      checkInOut += "12";
  //    //    }
  //    else if (valueSensorIn == 1 && valueSensorOut == 0) {
  //      checkInOut += "10";
  //    }
  //    tempValueSensorIn = valueSensorIn;
  //    tempValueSensorOut = valueSensorOut;
  //  }
  //  if (checkInOut.length() <= 4) {
  //    if (checkInOut == "0210" && slotParking > 0) {
  //      slotParking--;
  //      checkInOut = "";
  //      printLCD();
  //      myservo.write(servoClose);
  //    }
  //    else if (checkInOut == "1002") {
  //      slotParking++;
  //      checkInOut = "";
  //      printLCD();
  //      myservo.write(servoClose);
  //    }
  //    else if (checkInOut != "0210" && checkInOut != "1002" && checkInOut.length() == 4) {
  //      checkInOut = "";
  //      printLCD();
  //    }
  //  }
}

void printLCD() {
  lcd.clear();
  if (slotParking > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Quet The Tai Day");
    lcd.setCursor(0, 1);
    lcd.print("Cho Con Trong:" + String(slotParking));
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("Caution!");
    lcd.setCursor(0, 1);
    lcd.print("Bai Do Xe Da Day");
  }
}

void read_ESP() {
  while (mySerial.available()) {
    char inChar = (char)mySerial.read();
    statusFromESP += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
  if (stringComplete) {
    //    Serial.print("Data nhận: ");
    //    Serial.println(statusFromESP);
    //==============================
    //Xử lý dữ liệu
    handleData(statusFromESP);
    //==============================
    statusFromESP = "";
    stringComplete = false;
  }
}

void BarrierControl(String statusFromESP) {
  lcd.clear();
  CheckInOut();
  if (statusFromESP.indexOf("*") >= 0) {
    valueSensorIn = digitalRead(sensorIn);
    valueSensorOut = digitalRead(sensorOut);
    if (valueSensorIn == 0) {
      myservoIn.write(servoOpen);
    } else if (valueSensorOut == 0) {
      myservoOut.write(servoOpen);
    }
    closeBarrier = millis();
    lcd.setCursor(0, 0);
    lcd.print("The Hop Le");
    lcd.setCursor(0, 1);
    lcd.print("Wellcome!");
  }
  else if (statusFromESP.indexOf(")") >= 0) {
    valueSensorIn = digitalRead(sensorIn);
    valueSensorOut = digitalRead(sensorOut);
    if (valueSensorIn == 0) {
      myservoIn.write(servoClose);
    } else if (valueSensorOut == 0) {
      myservoOut.write(servoClose);
    }
    closeBarrier = millis();
    lcd.setCursor(0, 0);
    lcd.print("The Khong Hop Le");
    lcd.setCursor(0, 1);
    lcd.print("Vui Long Thu Lai");
    statusInOut = 0;
  }
  analogWrite(6, 230);
  buzzer = millis();
}

void handleData(String statusFromESP) {
  //  //Tìm vị trí kí tự
  //  int findStarChar, findOpenBracketChar = -1;
  //  findStarChar = statusFromESP.indexOf("*");
  //  findOpenBracketChar = statusFromESP.indexOf("(");
  //  //Cắt chuỗi
  //  if (findStarChar >= 0 && findOpenBracketChar >= 0) {
  //    statusFromESP =  statusFromESP.substring(findStarChar + 1, findOpenBracketChar);
  Serial.print("Data đã xử lý: ");
  Serial.println(statusFromESP);
  BarrierControl(statusFromESP);
}

void ScanCard() {
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    //    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  //  Serial.println();
  //  Serial.print("Message : ");
  content.toUpperCase();
  content = content.substring(1);
  //  Serial.println(content);
  sendUID(content);
}
void sendUID(String content) {
  UIDSend = "";
  //Đóng gói UID
  UIDSend = "*" + content + "(";
  //  Serial.print("Send to ESP: ");
  //  Serial.println(UIDSend);
  //  Serial.flush();
  mySerial.println(UIDSend);
  mySerial.flush();
}
