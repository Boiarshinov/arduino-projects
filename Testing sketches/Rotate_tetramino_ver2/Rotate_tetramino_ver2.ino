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
  int x;
  int y;
};

struct coord_float {
  float x;
  float y;
};

struct tetramino { //Структура для хранения различных типов тетрамино и их начальных координат
  byte number_of_type; //Пока этот параметр не используется, следует удалить его, если не будет исползоваться впредь
  char type;
  byte rotate_position; //Параметр, в котором хранится вращательное положение фигуры
  coord dots[4];
};

//Описание начальных положений фигур тетрамино
const tetramino figure[7] = {
  {0, 'i', 0, {{ 5, 4}, { 4, 4}, { 3, 4}, { 2, 4}}}, //Палка
  {1, 'o', 0, {{ 5, 3}, { 5, 4}, { 4, 3}, { 4, 4}}}, //Квадрат
  {2, 'l', 0, {{ 5, 4}, { 5, 3}, { 4, 3}, { 3, 3}}}, //L-палка
  {3, 'j', 0, {{ 5, 3}, { 5, 4}, { 4, 4}, { 3, 4}}}, //J-палка
  {4, 's', 0, {{ 5, 3}, { 5, 4}, { 4, 4}, { 4, 5}}}, //S-загогулина
  {5, 'z', 0, {{ 4, 2}, { 4, 3}, { 5, 3}, { 5, 4}}}, //Z-загогулина
  {6, 't', 0, {{ 5, 3}, { 5, 4}, { 5, 5}, { 4, 4}}}  //T-палка
};

//Матрица проворота
const coord_float rotate_centers_matrix[7][4] = {
  {{-1, 1}, {-0.5, -2.5}, {2, 1}, {-0.5, 0.5}}, //для палки
  {{-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}, {0.5, 0.5}}, //для квадрата
  {{ -1, 0}, {0.5, -1.5}, {1, 1}, { -0.5, 0.5}}, //для L-палки
  {{-0.5, 0.5}, {-1, -1}, {1.5, -0.5}, {0, 1}}, //для J-палки
  {{-0.5, 0.5}, {-1, 0}, {0.5, -1.5}, {1, 1}}, //для S-загогулины
  {{0, 1}, {-0.5, 0.5}, {-1, -1}, {1.5, -0.5}}, //для Z-загогулины
  {{-0.5, 0.5}, {-1, 0}, {0, -1}, {1.5, 0.5}}, //для T-палки
};

tetramino act_figure = figure[0]; //Переменная, в которой хранится текущая фигура

byte speed_of_game = 1; //Скорость игры
word time_before_next = 700; //Время между шагами

bool ruins[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

bool pole[8][8]; //Массив для хранения сведений о закрашенности поля

byte pole_byte[8]; //Массив, в котором содержатся сведения о закрашенности поля (в таком виде удобно выдавать в сдвиговой регистр)

bool last_state_rotate = false; //Переменная, в которой находится предыдущее состояние нажатия кнопки "Поворот"
bool last_state_next = false; //Переменная, в которой находится предыдущее состояние нажатия кнопки "Поворот"

void setup() {
  initialize(); //Инициализация пинов
  all_off(); //Выключение всех рядов и колонок, чтобы не отображалась всякая фигня
  new_figure(); //Разыгрываем новую фигуру
  actualize_pole(); //Закрашиваем поле в соответствии с расположением развалин и фигуры
  display_pole();
  //Serial.begin(9600);
}

void loop() {
  unsigned long time_for_next_step = millis() + time_before_next;
  while (millis() < time_for_next_step) //Ожидание нажатия одной из кнопок до окончания хода
  {
    if (!last_state_rotate && digitalRead(rotate_button_Pin) == HIGH)
    {
      rotate();
      last_state_rotate = true;
    }
    if (!last_state_next && digitalRead(start_pause_button_Pin) == HIGH)
    {
      new_figure();
      last_state_next = true;
    }
    actualize_pole();
    display_pole();

    if (digitalRead(rotate_button_Pin) == LOW)
    {
      last_state_rotate = false;
    }
    if (digitalRead(start_pause_button_Pin) == LOW)
    {
      last_state_next = false;
    }
  }
  actualize_pole();
  display_pole();
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

//Функция создающая новую фигуру
void new_figure()
{
  act_figure = figure[(act_figure.number_of_type + 1) % 7];
}

//Функция, поворачивающая фигуру
void rotate()
{
  float center_x = act_figure.dots[0].x + rotate_centers_matrix[act_figure.number_of_type][act_figure.rotate_position].x;
  float center_y = act_figure.dots[0].y + rotate_centers_matrix[act_figure.number_of_type][act_figure.rotate_position].y;

  for (byte i = 0; i < 4; i++)
  {
    int temp = act_figure.dots[i].x;
    act_figure.dots[i].x = center_y - act_figure.dots[i].y + center_x;
    act_figure.dots[i].y = temp - center_x + center_y;
  }
  act_figure.rotate_position = (act_figure.rotate_position + 1) % 4;
}

///Функция, которая закрашивает поле в соответствии с расположением актуального фрагмента
void actualize_pole()
{
  for (byte i = 0; i < 8; i++)
  {
    for (byte j = 0; j < 8; j++)
    {
      pole[i][j] = ruins[i][j];
    }
  }
  //Закрашивание точек, где находится фигура тетрамино
  for (byte i = 0; i < 4; i++)
  {
    if (act_figure.dots[i].x >= 0 && act_figure.dots[i].x <= 7) //Выводим только те точки фигуры, которые находятся в поле. Иначе возникнет ошибка
    {
      pole[act_figure.dots[i].x][act_figure.dots[i].y] = true;
    }
  }
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

void game_over()
{
  for (byte i = 0; i < 3; i++)
  {
    all_on();
    delay(300);
    all_off();
    delay(300);
  }

  new_figure();
  while (digitalRead(start_pause_button_Pin) == LOW)
  {
    display_pole();
  }
}
