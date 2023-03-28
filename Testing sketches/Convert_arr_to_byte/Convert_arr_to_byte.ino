void setup() {
  Serial.begin(9600);
  bool massive[8] = {1, 0, 0, 0, 0, 0, 1, 0};
  for (byte i = 0; i < 8; i++)
  {
    Serial.print(massive[i]);
  }
  Serial.println("");
  Serial.println(from_array_to_byte(massive), BIN);

  bool massive_2[2][8] = {{1, 0, 0, 0, 0, 0, 1, 0}, {1, 1, 1, 1, 1, 1, 1, 0}};
  Serial.println(from_array_to_byte(massive_2[1]), BIN);
}

void loop() {


}

byte from_array_to_byte(bool array_1[8])
{
  byte result = 0;
  for (byte i = 0; i < 8; i++)
  {
    result = result + array_1[i] * (round(pow(2, 7 - i)));
  }
  return result;
}
