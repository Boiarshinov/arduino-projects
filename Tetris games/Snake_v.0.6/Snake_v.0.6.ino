const byte clock_Pin = 4;
const byte row_latch_Pin = 5;
const byte row_data_Pin = 6;
const byte column_latch_Pin = 7;
const byte column_data_Pin = 8;
const byte left_button_Pin = A0;
const byte right_button_Pin = A3;
const byte start_pause_button_Pin = A2;
const byte rotate_button_Pin = A1;

#define left 1
#define right 2
#define forward 0


byte side;

#define north 1
#define east 2
#define south 3
#define west 4

byte way;

struct coord {
  byte x;
  byte y;
};

byte speed_of_game = 1;
word time_before_next = 700;

coord apple;

coord snake[64];

byte length_of_snake;

//Массив, в котором содержатся сведения о закрашенности поля
byte pole_byte[8] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

bool pole[8][8];

void setup() {
  //Инициализация пинов
  initialize();
  //Инициализация рандома
  randomSeed(analogRead(A5));
  //Выключение всех рядов и колонок, чтобы не отображалась всякая фигня
  all_off();


  //Инициализация змейки
  initialize_snake();
  //Задаем яблоко
  new_apple();

  actualize_pole();
  display_pole();
  
  //Serial.begin(9600);

  //Удерживаем, пока не будет нажата кнопка Старт
  while (digitalRead(start_pause_button_Pin) == LOW)
  {
    display_pole();
  }
}

void loop() {
  side = forward;
  //Проверка нажатия кнопки, запись нажатия
  unsigned long time_for_next_step = millis() + time_before_next;
  while (millis() < time_for_next_step)
  {
    if (digitalRead(right_button_Pin) == HIGH)
    {
      side = right;
    }
    if (digitalRead(left_button_Pin) == HIGH)
    {
      side = left;
    }
    /*
      if (digitalRead(start_pause_button_Pin) == HIGH)
      {
      delay(1000);
      while (digitalRead(start_pause_button_Pin) == LOW)
      {
        display_snake();
        display_apple();
      }
      time_for_next_step = millis() + time_before_next; //чтобы змейка не сразу заползла
      }
    */
    actualize_pole();
    display_pole();
  }
  //Перестройка змейки, когда прошло время шага
  step_of_snake();
  //Показываем змею и яблоко
  actualize_pole();
  display_pole();
  //Вывод в сериал порт
  //otladka();
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
  pinMode(rotate_button_Pin, INPUT);
  pinMode(start_pause_button_Pin, INPUT);
  pinMode(right_button_Pin, INPUT);
  pinMode(left_button_Pin, INPUT);
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

void initialize_snake()
{
  length_of_snake = 3;
  snake[0].x = 4;
  snake[0].y = 4;
  snake[1].x = 4;
  snake[1].y = 3;
  snake[2].x = 4;
  snake[2].y = 2;
  for (byte i = length_of_snake; i < 64; i++)
    {
    snake[i].x = -1;
    snake[i].y = -1;
    }
  way = east;
}


void step_of_snake()
{
  //объявление временной переменной
  coord temp;
  //определение стороны, куда поползет змея
  if (side == left)
  {
    way = way - 1;
    if (way == 0) {
      way = west;
    }
  }
  if (side == right)
  {
    way = way + 1;
    if (way == 5) {
      way = north;
    }
  }
  //Присвоение координаты голове змеи
  switch (way)
  {
    case north:
      {
        temp.x = snake[0].x - 1;
        temp.y = snake[0].y;
        break;
      }
    case east:
      {
        temp.x = snake[0].x;
        temp.y = snake[0].y + 1;
        break;
      }
    case south:
      {
        temp.x = snake[0].x + 1;
        temp.y = snake[0].y;
        break;
      }
    case west:
      {
        temp.x = snake[0].x;
        temp.y = snake[0].y - 1;
        break;
      }
  }
  temp.x = temp.x % 8;
  temp.y = temp.y % 8;


  //если змея съест яблоко, то увеличиваем ее длину
  if (apple.x == temp.x && apple.y == temp.y)
  {
    length_of_snake++;
  }
  //Сдвигание всех частей змейки кроме первой
  for (byte i = length_of_snake - 1; i > 0; i--)
  {
    snake[i] = snake[i - 1];
  }
  snake[0].x = temp.x;
  snake[0].y = temp.y;

  for (byte i = length_of_snake - 1; i > 0; i--)
  {
    if (snake[i].x == snake[0].x && snake[i].y == snake[0].y)
    {
      game_over();
    }
  }
  //если змея съела яблоко, то определяем новое яблоко
  if (apple.x == temp.x && apple.y == temp.y)
  {
    new_apple();
  }
}

void new_apple()
{
  byte x = random(0, 8); //Рандомно выбираем координаты нового яблока
  byte y = random(0, 8);
  apple = {x, y};
  //Проверка, попало ли яблоко в змею
  for (byte i = 0; i < length_of_snake; i++)
  {
    if (apple.x == snake[i].x && apple.y == snake[i].y)
    {
      new_apple();
    }
  }
}

void actualize_pole()
{
  //Обнуление всего поля
  for (byte i = 0; i < 8; i++)
  {
    for (byte j = 0; j < 8; j++)
    {
      pole[i][j] = false;
    }
  }

  //Закрашивание точек, где находится змея
  for (byte i = 0; i < length_of_snake; i++)
  {
    pole[snake[i].x][snake[i].y] = true;
  }

  //Закрашивание точки, где находится яблоко
  pole[apple.x][apple.y] = true;
}

void display_pole()
{
  for (byte i = 0; i < 8; i++)
  {
    all_off();
    row_register_Write(round(pow(2, i))); //округление нужно для корректного приведения типов
    column_register_Write(from_array_to_byte(pole[i]));
    delayMicroseconds(100);
  }
}

void game_over()
{
  for (byte i = 0; i < 3; i++)
  {
    all_on();
    delay(300);
    all_off();
    delay(300);
  }
  initialize_snake();
  new_apple();
  while (digitalRead(start_pause_button_Pin) == LOW)
  {
    actualize_pole();
    display_pole();
  }
}

byte from_array_to_byte(bool array_1[8]) //Функция, преобразующая массив из 8 бит в байт
{
  byte result = 0;
  for (byte i = 0; i < 8; i++)
  {
    result = result + array_1[i] * (round(pow(2, 7 - i)));
  }
  return result;
}

void otladka()
{
  Serial.println("/////////////////////");
  Serial.print("Snake's side = ");
  switch (side)
  {
    case left:
      {
        Serial.println("left");
        break;
      }
    case right:
      {
        Serial.println("right");
        break;
      }
    case forward:
      {
        Serial.println("forward");
        break;
      }
  }
  Serial.println("");

  Serial.print("Snake's way = ");
  switch (way)
  {
    case north:
      {
        Serial.println("north");
        break;
      }
    case east:
      {
        Serial.println("east");
        break;
      }
    case south:
      {
        Serial.println("south");
        break;
      }
    case west:
      {
        Serial.println("west");
        break;
      }
  }
  Serial.println("");

  Serial.println("Snake's coordinate");
  for (byte i = 0; i < length_of_snake; i++)
  {
    Serial.print(i);
    Serial.print("   x = ");
    Serial.print(snake[i].x);
    Serial.print("   y = ");
    Serial.println(snake[i].y);
  }
  Serial.println("");
}
