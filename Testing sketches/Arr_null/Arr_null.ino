void setup() {
  Serial.begin(9600);
  bool massive[8] = {1, 0, 1, 1, 0, 0, 1, 0};
  for (byte i = 0; i < 8; i++)
  {
    Serial.print(massive[i]);
  }
  Serial.println("");

  for (byte i = 0; i < 8; i++)
  {
    massive[i] = false;
  }

  for (byte i = 0; i < 8; i++)
  {
    Serial.print(massive[i]);
  }
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:

}
