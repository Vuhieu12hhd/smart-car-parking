
int sensorIn = 7;
int sensorOut = 8;

int valueSensorIn;
int valueSensorOut;

int statusInOut = 0;

void setup()
{
  Serial.begin(115200);

  pinMode(sensorIn, INPUT);
  pinMode(sensorOut, INPUT);
}

void loop()
{
  valueSensorIn = digitalRead(sensorIn);
  valueSensorOut = digitalRead(sensorOut);
  if (valueSensorIn == 0) {
    statusInOut = 1;
  } else if (valueSensorOut == 0) {
    statusInOut = 0;
  }

  Serial.print("In: ");
  Serial.print(valueSensorIn);
  Serial.print(" Out: ");
  Serial.println(valueSensorOut);
  if (statusInOut == 0) {
    Serial.println("In");
  } else if (statusInOut == 1) {
    Serial.println("Out");
  }
  delay(200);
}
