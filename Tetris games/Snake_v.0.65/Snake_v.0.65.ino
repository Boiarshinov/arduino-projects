const byte clock_Pin = 4;
const byte row_latch_Pin = 5;
const byte row_data_Pin = 6;
const byte column_latch_Pin = 7;
const byte column_data_Pin = 8;
const byte left_button_Pin = A0;
const byte right_button_Pin = A3;
const byte start_pause_button_Pin = A2;
const byte rotate_button_Pin = A1;

struct coord { //Структура для хранения координат
  byte x;
  byte y;
};

///Параметры змейки
coord snake[64]; //Переменная, в которой хранятся координаты отсеков змейки
byte length_of_snake; //Длина змейки

//Сторона света, в которую двигается змейка
byte way;
#define north 1
#define east 2
#define south 3
#define west 4

//Направление движения змейки, относительно предыдущего положения
byte side;
#define left 1
#define right 2
#define forward 0
///

bool flag_rotate = false; //Флаг для разворота змеи в начале перестроения

coord apple; //Переменная, в которой хранятся координаты яблока

byte speed_of_game = 1; //Скорость игры
word time_before_next = 700; //Время между шагами

bool pole[8][8]; //Массив для хранения сведений о закрашенности поля

byte pole_byte[8] = { //Массив, в котором содержатся сведения о закрашенности поля (в таком виде удобно выдавать в сдвиговой регистр)
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

///Выполняется при подаче питания
void setup() {
  initialize(); //Инициализация пинов
  randomSeed(analogRead(A5)); //Инициализация рандома
  all_off(); //Выключение всех рядов и колонок, чтобы не отображалась всякая фигня
  initialize_snake(); //Инициализация змейки
  new_apple(); //Задаем яблоко
  actualize_pole(); //Закрашиваем поле в соответствии с расположением змеи и яблока
  display_pole(); //Выводим на экран текущую окраску поля
  //Serial.begin(9600);
  while (digitalRead(start_pause_button_Pin) == LOW) //Удерживаем, пока не будет нажата кнопка Старт
  {
    display_pole();
  }
}

///Выполняется циклически
void loop() {
  side = forward; //Обновление направления движения змеи в начале хода
  flag_rotate = false;
  //Проверка нажатия кнопки, запись нажатия
  unsigned long time_for_next_step = millis() + time_before_next;
  while (millis() < time_for_next_step) //Ожидание нажатия одной из кнопок до окончания хода
  {
    if (digitalRead(right_button_Pin) == HIGH)
    {
      side = right;
    }
    if (digitalRead(left_button_Pin) == HIGH)
    {
      side = left;
    }
    if (digitalRead(rotate_button_Pin) == HIGH)
    {
      flag_rotate = true;
    }
    actualize_pole();
    display_pole();
  }
  if (flag_rotate)
  {
    rotate();
  }
  //Перестройка змейки, когда прошло время шага
  step_of_snake();
  //Показываем змею и яблоко
  actualize_pole();
  display_pole();
  //Вывод в сериал порт
  //otladka();
}

///Инициализация пинов
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

///Функция создания змеи из трех секций по центру поля
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

///Функция, рассчитывающая шаг змеи и ее взаимодействие с объектами
void step_of_snake()
{  
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
  
  coord temp; //объявление временной переменной
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

  //если змея съест яблоко, то увеличиваем ее длину и создаем новое яблоко
  if (apple.x == temp.x && apple.y == temp.y)
  {
    length_of_snake++;
    new_apple();
  }
  
  //Сдвигание всех частей змейки кроме первой
  for (byte i = length_of_snake - 1; i > 0; i--)
  {
    snake[i] = snake[i - 1];
  }
  snake[0].x = temp.x;
  snake[0].y = temp.y;

  //Проверка самопересечения змеи
  for (byte i = length_of_snake - 1; i > 0; i--)
  {
    if (snake[i].x == snake[0].x && snake[i].y == snake[0].y)
    {
      game_over();
    }
  }
}

///Функция, разворачивающая направление движения змеи
void rotate()
{
  //Переворот фрагментов змейки
  for (byte i = 0; i < (length_of_snake + 1) / 2; i++)
  {
    coord temp = snake[i];
    snake[i] = snake[length_of_snake - 1 - i];
    snake[length_of_snake - 1 - i] = temp;
  }
  
  //Изменение направления движения змейки
  switch (way)
  {
    case north:
      {
        way = south;
        break;
      }
    case east:
      {
        way = west;
        break;
      }
    case south:
      {
        way = north;
        break;
      }
    case west:
      {
        way = east;
        break;
      }
  }
}

///Функция создания на поле нового яблока
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

///Функция, которая закрашивает поле в соответствии с расположением змеи и яблока
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

///Функция вывода на светодиодную матрицу
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

///Функция, преобразующая массив из 8 бит в байт
byte from_array_to_byte(bool array_1[8]) 
{
  byte result = 0;
  for (byte i = 0; i < 8; i++)
  {
    result = result + array_1[i] * (round(pow(2, 7 - i))); //округление нужно для корректного приведения типов
  }
  return result;
}

///Функция записи в сдвиговой регистр, отвечающий за ряды
void row_register_Write(byte number)
{
  digitalWrite(row_latch_Pin, LOW); //Открытие защелки
  shiftOut(row_data_Pin, clock_Pin, MSBFIRST, number);
  digitalWrite(row_latch_Pin, HIGH); //Закрытие защелки
}

///Функция записи в сдвиговой регистр, отвечающий за колонки
void column_register_Write(byte number)
{
  number = B11111111 - number; //инвертируем значение бита
  digitalWrite(column_latch_Pin, LOW); //Открытие защелки
  shiftOut(column_data_Pin, clock_Pin, LSBFIRST, number);
  digitalWrite(column_latch_Pin, HIGH); //Закрытие защелки
}

///Функция обесточивания всех светодиодов
void all_off()
{
  row_register_Write(B00000000);
  column_register_Write(B00000000);
}

///Функция зажигания всех светодиодов
void all_on()
{
  row_register_Write(B11111111);
  column_register_Write(B11111111);
}

///Функция конца игры (мигание всем экраном 3 раза и создание новой игры)
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

///Функция, использующаяся для отладки игры
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
