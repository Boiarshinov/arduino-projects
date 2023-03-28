String temp_str;

int second;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  //Serial.print("First = ");
  int first = 0;
  while (Serial.available() > 0)
  {
    first = first * 10 + (Serial.read() - '0');
    Serial.print("Received = ");
    Serial.println(first);
    Serial.println();
  }



  /*
    Serial.print("First = ");
    while (Serial.available()) first = first * 10 + (Serial.read() - '0');

    Serial.print("Second = ");
    while (Serial.available()) second = second * 10 + (Serial.read() - '0');

    Serial.print("Sum = ");
    Serial.println(first + second);
  */
}
