//======================FreeRTOS======================
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>


TaskHandle_t xTaskCloseBarrier;
TaskHandle_t xTaskScanCardCount;
TaskHandle_t xTaskReadESP;
SemaphoreHandle_t xBinarySemaphore;

//======================Define task======================
void TaskCloseBarrier(void *pvParameters);
void TaskScanCardCount(void *pvParameters);
void TaskReadESP(void *pvParameters);

//======================EEPROM======================
#include <EEPROM.h>
int address = 0;
//======================UART======================
#include <SoftwareSerial.h>
const byte RX = 2;
const byte TX = 3;
SoftwareSerial mySerial = SoftwareSerial(RX, TX);
String statusFromESP = "";
bool stringComplete = false;
//======================Milis======================
long last = 0;
long scanCard = 0;
long buzzer = 0;
long closeBarrier = 0;
//======================RFID======================
#include <SPI.h>
#include <MFRC522.h>
String UIDSend = "";
//======================LCD I2C======================
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//======================Servo======================
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
  lcd.init(); //Khởi động màn hình. Bắt đầu cho phép Arduino sử dụng màn hình
  lcd.backlight(); //Bật đèn nền
  printLCD();
  Serial.begin(9600);
  mySerial.begin(9600);
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

  //======================FreeRTOS======================
  xTaskCreate(TaskCloseBarrier, "TaskCloseBarrier", 128, NULL, 1, &xTaskCloseBarrier);
  xTaskCreate(TaskScanCardCount, "TaskScanCardCount", 128, NULL, 1, &xTaskScanCardCount);
  xTaskCreate(TaskReadESP, "TaskReadESP", 128, NULL, 1, &xTaskReadESP);
  if (xBinarySemaphore == NULL)
  {
    xBinarySemaphore = xSemaphoreCreateBinary();
    if (xBinarySemaphore != NULL)
      xSemaphoreGive(xBinarySemaphore);
  }
  vTaskStartScheduler();
}
void loop()
{
}

void TaskCloseBarrier(void *pvParameters) {
  //======================Close Barrier======================
  while (1) {
    if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdPASS)
    {
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
        printLCD();
      }
      //    Serial.println("TaskCloseBarrier");
      xSemaphoreGive(xBinarySemaphore);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskScanCardCount(void *pvParameters) {
  //======================Scan card======================
  while (1) {
    if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdPASS)
    {
      if (millis() - scanCard >= 1000) {
        valueSensorOut = digitalRead(sensorOut);
        if (slotParking > 0 || valueSensorOut == 0) {
          ScanCard();
          scanCard = millis();
        }
      }
      xSemaphoreGive(xBinarySemaphore);
    }
    //    Serial.println("TaskScanCardCount");
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskReadESP(void *pvParameters) {
  //======================Read status from ESP======================
  while (1) {
    if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdPASS)
    {
          read_ESP();
          xSemaphoreGive(xBinarySemaphore);
    }
    //    Serial.println("TaskReadESP");
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void CheckInOut() {
  valueSensorIn = digitalRead(sensorIn);
  valueSensorOut = digitalRead(sensorOut);
  if (valueSensorIn == 0) {
    statusInOut = 1;
  } else if (valueSensorOut == 0) {
    statusInOut = -1;
  }
}

void printLCD() {
  lcd.clear(); //xóa màn hình cho vòng lặp kế tiếp 
  if (slotParking > 0) {
    lcd.setCursor(0, 0); //xét hàng cần hiển thị
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
    //==================================
    //Xử lý dữ liệu
    handleData(statusFromESP);
    //==================================
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
    lcd.print("Mo Rao!");
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
  //======================Packing UID======================
  UIDSend = "*" + content + "(";
  //  Serial.print("Send to ESP: ");
  //  Serial.println(UIDSend);
  //  Serial.flush();
  mySerial.println(UIDSend);
  mySerial.flush();
}
