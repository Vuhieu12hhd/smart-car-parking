//FreeRTOS
#include <Arduino_FreeRTOS.h>
#include <task.h>

TaskHandle_t xTaskCloseBarrier;
TaskHandle_t xTaskScanCardCount;
TaskHandle_t xTaskTurnOffBuzzer;
TaskHandle_t xTaskReadESP;
TaskHandle_t xTaskCheckInOut;
TaskHandle_t xTaskPrintLCD;
TaskHandle_t xTaskRead_ESP;
TaskHandle_t xTaskScanCard;



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
  Serial.begin(115200);
  mySerial.begin(115200);
  last = millis();
  scanCard = millis();
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

  //FreeRTOS
  xTaskCreate(TaskCloseBarrier, "TaskCloseBarrier", 100, NULL, 2, &xTaskCloseBarrier);
  xTaskCreate(TaskScanCardCount, "TaskScanCardCount", 100, NULL, 2, &xTaskScanCardCount);
  xTaskCreate(TaskTurnOffBuzzer, "TaskTurnOffBuzzer", 100, NULL, 2, &xTaskTurnOffBuzzer);
  xTaskCreate(TaskReadESP, "TaskReadESP", 64, NULL, 2, &xTaskReadESP);
  xTaskCreate(TaskCheckInOut, "TaskCheckInOut", 64, NULL, 1, &xTaskCheckInOut);
  xTaskCreate(TaskPrintLCD, "TaskPrintLCD", 64, NULL, 1, &xTaskPrintLCD);
  xTaskCreate(TaskRead_ESP, "TaskRead_ESP", 64, NULL, 1, &xTaskRead_ESP);
  xTaskCreate(TaskScanCard, "TaskScanCard", 64, NULL, 1, &xTaskScanCard);

  delay(10);
  vTaskStartScheduler();
}
void loop()
{

}

void TaskCloseBarrier(void *px) {
  //Đóng rào
  if (millis() - closeBarrier == 5000) {
    slotParking -= statusInOut;
    EEPROM.write(address, slotParking);
    if (statusInOut == 1) {
      myservoIn.write(servoClose);
    }
    else if (statusInOut == -1) {
      myservoOut.write(servoClose);
    }
    statusInOut = 0;
    vTaskPrioritySet(xTaskPrintLCD, 3);

  }
  Serial.println("TaskCloseBarrier run!");
}

void TaskScanCardCount(void *px) {
  //Quẹt thẻ
  if (millis() - scanCard >= 1000) {
    valueSensorOut = digitalRead(sensorOut);
    if (slotParking > 0 || valueSensorOut == 0) {
      vTaskPrioritySet(xTaskScanCard, 3);
      scanCard = millis();
    }
  }
  Serial.println("TaskScanCardCount run!");
}

void TaskTurnOffBuzzer(void *px) {
  //Tắt còi
  if (millis() - buzzer >= 200) {
    analogWrite(6, 0);
  }
  Serial.println("TaskTurnOffBuzzer run!");
}

void TaskReadESP(void *px) {
  //Read status from ESP:
  vTaskPrioritySet(xTaskRead_ESP, 3);
  Serial.println("TaskReadESP run!");
}
void TaskCheckInOut(void *px) {

  valueSensorIn = digitalRead(sensorIn);
  valueSensorOut = digitalRead(sensorOut);
  if (valueSensorIn == 0) {
    statusInOut = 1;
  } else if (valueSensorOut == 0) {
    statusInOut = -1;
  }
}

void TaskPrintLCD(void *px) {
  lcd.clear();
  if (slotParking > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Hay quet the!");
    lcd.setCursor(0, 1);
    lcd.print("Cho con trong:" + String(slotParking));
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("Xin loi!");
    lcd.setCursor(0, 1);
    lcd.print("Bai do xe da day");
  }
}

void printLCD() {
  lcd.clear();
  if (slotParking > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Hay quet the!");
    lcd.setCursor(0, 1);
    lcd.print("Cho con trong:" + String(slotParking));
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("Xin loi!");
    lcd.setCursor(0, 1);
    lcd.print("Bai do xe da day");
  }
}

void TaskRead_ESP(void *px) {
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
    HandleData(statusFromESP);
    //==============================
    statusFromESP = "";
    stringComplete = false;
  }
}

void BarrierControl(String statusFromESP) {
  lcd.clear();
  vTaskPrioritySet(xTaskCheckInOut, 3);
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
    lcd.print("The hop le");
    lcd.setCursor(0, 1);
    lcd.print("Mo rao!");
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
    lcd.print("The khong hop le");
    lcd.setCursor(0, 1);
    lcd.print("Dong rao!");
    statusInOut = 0;
  }
  analogWrite(6, 230);
  buzzer = millis();
}

void HandleData(String statusFromESP) {
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

void TaskScanCard(void *px) {
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
  SendUID(content);
}
void SendUID(String content) {
  UIDSend = "";
  //Đóng gói UID
  UIDSend = "*" + content + "(";
  //  Serial.print("Send to ESP: ");
  //  Serial.println(UIDSend);
  //  Serial.flush();
  mySerial.println(UIDSend);
  mySerial.flush();
}
