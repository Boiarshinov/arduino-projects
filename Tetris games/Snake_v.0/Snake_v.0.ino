#define left 1
#define right 2
#define forward 0

byte side;

#define north 1
#define east 2
#define south 3
#define west 4

byte way;

int row[] = {A1, 10, 7, 12, A4, 6, A5, 4};
int column[] = {11, 2, 3, A0, 5, 13, 9, 8};

struct coord {
  byte x;
  byte y;
};

byte speed_of_game = 1;
word time_before_next = 700;

coord apple;

coord snake[64];

byte length_of_snake = 3;

void setup() {
  //Инициализация пинов
  initialize();
  //Инициализация рандома
  randomSeed(analogRead(A5));
  //Выключение всех рядов и колонок, чтобы не отображалась всякая фигня
  all_off();
  //Инициализация змейки
  initialize_snake();
  //Показываем змею
  display_snake();
  //Задаем яблоко, но не показываем его
  new_apple();

  //Serial.begin(9600);
}

void loop() {
  side = forward;
  //Проверка нажатия кнопки, запись нажатия
  unsigned long time_for_next_step = millis() + time_before_next;
  while (millis() < time_for_next_step)
  {
    if (digitalRead(A2) == HIGH)
    {
      side = right;
    }
    if (digitalRead(A3) == HIGH)
    {
      side = left;
    }
    display_snake();
    display_apple();
  }
  //Перестройка змейки, когда прошло время шага
  step_of_snake();
  //Показываем змею и яблоко
  display_snake();
  display_apple();
  //Вывод в сериал порт
  //otladka();
}

void initialize()
{
  for (byte i = 0; i < 8; i++)
  {
    pinMode(row[i], OUTPUT);
    pinMode(column[i], OUTPUT);
  }
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
}

void all_off()
{
  for (byte i = 0; i < 8; i++)
  {
    digitalWrite(row[i], LOW);
    digitalWrite(column[i], HIGH);
  }
}

void all_on()
{
  for (byte i = 0; i < 8; i++)
  {
    digitalWrite(row[i], HIGH);
    digitalWrite(column[i], LOW);
  }
}

void dot_on(coord dot)
{
  digitalWrite(row[dot.x], HIGH);
  digitalWrite(column[dot.y], LOW);
}

void dot_off(coord dot)
{
  digitalWrite(row[dot.x], LOW);
  digitalWrite(column[dot.y], HIGH);
}

void initialize_snake()
{
  snake[0].x = 4;
  snake[0].y = 4;
  snake[1].x = 4;
  snake[1].y = 3;
  snake[2].x = 4;
  snake[2].y = 2;
  for (byte i = 3; i < 64; i++)
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

void display_snake()
{
  all_off();
  for (byte i = 0; i < length_of_snake; i++)
  {
    dot_on(snake[i]);
    delayMicroseconds(1);
    dot_off(snake[i]);
  }
}

void new_apple()
{
  byte x = random(0, 8);
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

void display_apple()
{
  dot_on(apple);
  delayMicroseconds(10);
  dot_off(apple);
}

void game_over()
{
  for (byte i = 0; i<3; i++)
  {
    all_on();
    delay(300);
    all_off();
    delay(300);
  }
  while(1)
  {
  }
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

