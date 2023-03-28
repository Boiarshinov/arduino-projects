const byte clock_Pin = 13;
const byte row_latch_Pin = 12;
const byte row_data_Pin = 11;
const byte column_latch_Pin = 10;
const byte column_data_Pin = 9;

//Массив, в котором содержатся сведения о закрашенности поля
/*byte pole[8] = {
  B01010101,
  B10101010,
  B01010101,
  B10101010,
  B01010101,
  B10101010,
  B01010101,
  B10101010
};*/

byte pole[8] = { //Отладочное поле, закрашенное в трех углах
  B10000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B10000001
};

void setup() {
  //Инициализация пинов
  initialize();
  //Выключение всех рядов и колонок, чтобы не отображалась всякая фигня
  all_off();
  while (true)
  {
    display_pole();
  }
}

void loop() {

}

void row_register_Write(byte number)
{
  digitalWrite(row_latch_Pin, LOW);
  shiftOut(row_data_Pin, clock_Pin, MSBFIRST, number);
  digitalWrite(row_latch_Pin, HIGH);
}

void column_register_Write(byte number)
{
  number = B11111111 - number; //инвертируем значение бита
  digitalWrite(column_latch_Pin, LOW);
  shiftOut(column_data_Pin, clock_Pin, LSBFIRST, number);
  digitalWrite(column_latch_Pin, HIGH);
}

void initialize()
{
  pinMode(clock_Pin, OUTPUT);
  pinMode(row_latch_Pin, OUTPUT);
  pinMode(row_data_Pin, OUTPUT);
  pinMode(column_latch_Pin, OUTPUT);
  pinMode(column_data_Pin, OUTPUT);
}

void all_off()
{
  row_register_Write(B00000000);
  column_register_Write(B00000000);
}

void all_on()
{
  row_register_Write(B11111111);
  column_register_Write(B11111111);
}

void display_pole()
{
  for (byte i = 0; i < 8; i++)
  {
    all_off();
    row_register_Write(round(pow(2, i))); //округление нужно для корректного приведения типов
    column_register_Write(pole[i]);
    delayMicroseconds(1000);
  }
}
