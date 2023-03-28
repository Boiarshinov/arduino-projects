const byte clock_Pin = 4;
const byte row_latch_Pin = 5;
const byte row_data_Pin = 6;
const byte column_latch_Pin = 7;
const byte column_data_Pin = 8;
const byte left_button_Pin = A0;
const byte right_button_Pin = A3;
const byte start_pause_button_Pin = A2;
const byte rotate_button_Pin = A1;

const byte left = 0;
const byte right = 1;
const byte down = 2;

struct coord { //Структура для хранения координат
  int x;
  int y;
};

struct tetramino { //Структура для хранения различных типов тетрамино и их начальных координат
  byte number_of_type; //Пока этот параметр не используется, следует удалить его, если не будет исползоваться впредь
  char type;
  byte rotate_position; //Параметр, в котором хранится вращательное положение фигуры
  coord dots[4];
};

//Описание начальных положений фигур тетрамино
const tetramino figure[7] = {
  {0, 'i', 0, {{ -1, 4}, { -2, 4}, { -3, 4}, { -4, 4}}}, //Палка
  {1, 'o', 0, {{ -1, 3}, { -1, 4}, { -2, 3}, { -2, 4}}}, //Квадрат
  {2, 'l', 0, {{ -1, 4}, { -1, 3}, { -2, 3}, { -3, 3}}}, //L-палка
  {3, 'j', 0, {{ -1, 3}, { -1, 4}, { -2, 4}, { -3, 4}}}, //J-палка
  {4, 's', 0, {{ -1, 3}, { -1, 4}, { -2, 4}, { -2, 5}}}, //S-загогулина
  {5, 'z', 0, {{ -2, 2}, { -2, 3}, { -1, 3}, { -1, 4}}}, //Z-загогулина
  {6, 't', 0, {{ -1, 3}, { -1, 4}, { -1, 5}, { -2, 4}}}  //T-палка
};

//Матрица проворота
const coord rotate_matrix[7][4][4] = {
  {{{0, -1}, {1, 0}, {2, 1}, {3, 2}}, {{0, 1}, { -1, 0}, { -2, -1}, { -3, -2}}, {{0, -1}, {1, 0}, {2, 1}, {3, 2}}, {{0, 1}, { -1, 0}, { -2, -1}, { -3, -2}}},
  {{{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},
  {{{ -1, 1}, {0, 2}, {1, 1}, {2, 0}}, {{ -1, -2}, { -2, -1}, { -1, 0}, {0, 1}}, {{2, 0}, {1, -1}, {0, 0}, { -1, 1}}, {{0, 1}, {1, 0}, {0, -1}, { -1, -2}}},
  {{{ -1, -1}, {0, -2}, {1, -1}, {2, 0}}, {{ -1, 2}, { -2, 1}, { -1, 0}, {0, -1}}, {{2, 0}, {1, 1}, {0, 0}, { -1, -1}}, {{0, -1}, {1, 0}, {0, 1}, { -1, 2}}},
  {{{0, 1}, { -1, 0}, {0, -1}, { -1, -2}}, {{0, -1}, {1, 0}, {0, 1}, {1, 2}}, {{0, 1}, { -1, 0}, {0, -1}, { -1, -2}}, {{0, -1}, {1, 0}, {0, 1}, {1, 2}}},
  {{{1, 0}, {0, -1}, { -1, 0}, { -2, -1}}, {{ -1, 0}, {0, 1}, {1, 0}, {2, 1}}, {{1, 0}, {0, -1}, { -1, 0}, { -2, -1}}, {{ -1, 0}, {0, 1}, {1, 0}, {2, 1}}},
  {{{0, 1}, { -1, 0}, { -2, -1}, {0, -1}}, {{ -1, 1}, {0, 0}, {1, -1}, {1, 1}}, {{ -1, -1}, {0, 0}, {1, 1}, { -1, 1}}, {{2, -1}, {1, 0}, {0, 1}, {0, -1}}}
};

tetramino act_figure; //Переменная, в которой хранится текущая фигура

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

//Флаги предыдущего состояния кнопок
bool last_state_rotate = false; //Переменная, в которой находится предыдущее состояние нажатия кнопки "Поворот"
bool last_state_left = false; //Переменная, в которой находится предыдущее состояние нажатия кнопки "Налево"
bool last_state_right = false; //Переменная, в которой находится предыдущее состояние нажатия кнопки "Направо"
bool last_state_start_pause = false; //Переменная, в которой находится предыдущее состояние нажатия кнопки "Старт/Пауза"

void setup() {
  initialize(); //Инициализация пинов
  randomSeed(analogRead(A5)); //Инициализация рандома
  all_off(); //Выключение всех рядов и колонок, чтобы не отображалась всякая фигня
  delayMicroseconds(1000); // Даем время на инициализацию рандома, иначе будут генерироваться всегда палки первыми
  generate_ruins(); //Генерируем руины
  new_figure(); //Разыгрываем новую фигуру
  actualize_pole(); //Закрашиваем поле в соответствии с расположением развалин и фигуры

  //Serial.begin(9600);

  //Ожидаем нажатия СТАРТ
  while (digitalRead(start_pause_button_Pin) == LOW) //Удерживаем, пока не будет нажата кнопка Старт
  {
    display_pole();
  }
  last_state_start_pause = true;
}

void loop() {
  unsigned long time_for_next_step = millis() + time_before_next;
  while (millis() < time_for_next_step) //Ожидание нажатия одной из кнопок до окончания хода
  {
    if (!last_state_right && digitalRead(right_button_Pin) == HIGH)
    {
      step_right();
      actualize_pole();
      display_pole();
      last_state_right = true;
    }
    if (!last_state_left && digitalRead(left_button_Pin) == HIGH)
    {
      step_left();
      actualize_pole();
      display_pole();
      last_state_left = true;
    }
    if (!last_state_rotate && digitalRead(rotate_button_Pin) == HIGH)
    {
      rotate();
      last_state_rotate = true;
    }
    if (!last_state_start_pause && digitalRead(start_pause_button_Pin) == HIGH)
    {
      last_state_start_pause = true;
      unsigned long delay_time = millis() + time_before_next;
       while (millis() < delay_time)
      {
        display_pole();
      }
      while (digitalRead(start_pause_button_Pin) == LOW)
      {
        display_pole();
      }
      time_for_next_step = millis() + time_before_next; //чтобы шаг произошел не сразу
    }

    actualize_pole();
    display_pole();

    //Обнуление флагов при отжатии кнопок
    if (digitalRead(right_button_Pin) == LOW)
    {
      last_state_right = false;
    }
    if (digitalRead(left_button_Pin) == LOW)
    {
      last_state_left = false;
    }
    if (digitalRead(rotate_button_Pin) == LOW)
    {
      last_state_rotate = false;
    }
  }
  if (digitalRead(start_pause_button_Pin) == LOW) //Этот кусок вынесен за цикл шага, чтобы после выполнение setup прошел хотя бы один шаг без пауз
  {
    last_state_start_pause = false;
  }
  step_down();
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
  act_figure = figure[random(0, 7)];
}

///Функция шага фигуры вниз
void step_down()
{
  if (intersection(down)) //Если следующим шагом фигура пересечет поле
  {
    ruins_add_fr(); //Присоединение фигуры к полю

    //Проверка заполненности рядов, удаление лишнего
    byte full_rows_temp = full_rows();
    if (full_rows_temp != 0)
    {
      //Начинаем рассмотрение с последнего бита в байте,
      //при нахождении полного ряда записываем на сколько сдвигать каждый последующий ряд
      byte slide[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      for (int i = 7; i >= 0; i--)
      {
        if (full_rows_temp % 2 == 1)
        {
          slide[i] = 0; //Обнуление значения сдвига для уничтожаемых рядов
          for (int j = 0; j <= i - 1; j++)
          {
            slide[j]++;
          }
        }
        full_rows_temp = full_rows_temp >> 1;
      }
      //Мигаем уничтоженными рядами

      //Сдвигаем ряды на вычисленную дальность
      for (int i = 7; i >= 0; i--)
      {
        for (byte j = 0; j < 8; j++)
        {
          ruins[i + slide[i]][j] = ruins[i][j];
        }
      }
      //Обнуление всего, что выше сдвинутых рядов
      for (byte i = 0; i < slide[0]; i++)
      {
        for (byte j = 0; j < 8; j++)
        {
          ruins[i][j] = false;
        }
      }
    }
    new_figure(); //Создание новой фигуры
  }
  else {
    for (byte i = 0; i < 4; i++)
    {
      act_figure.dots[i].x += 1;
    }
  }
}

///Функция шага фигуры вправо
void step_right()
{
  if (!intersection(right))
  {
    for (byte i = 0; i < 4; i++)
    {
      act_figure.dots[i].y += 1;
    }
  }
}

///Функция шага фигуры влево
void step_left()
{
  if (!intersection(left))
  {
    for (byte i = 0; i < 4; i++)
    {
      act_figure.dots[i].y -= 1;
    }
  }
}

//Функция, поворачивающая фигуру
void rotate()
{
  tetramino temp_figure = act_figure; //Временная переменная для хранения перевернутой на 90 град. фигуры
  //Поворот временной переменной
  for (byte i = 0; i < 4; i++)
  {
    temp_figure.dots[i].x += rotate_matrix[temp_figure.number_of_type][temp_figure.rotate_position][i].x;
    temp_figure.dots[i].y += rotate_matrix[temp_figure.number_of_type][temp_figure.rotate_position][i].y;
  }
  temp_figure.rotate_position = (temp_figure.rotate_position + 1) % 4;

  bool no_intersection = true;
  //Проверка пересечения повернутой фигуры развалин или ее выхода за границы поля
  for (byte i = 0; i < 4; i++)
  {
    if (temp_figure.dots[i].y < 0 || temp_figure.dots[i].y > 7)
    {
      no_intersection = false;
    } else if (temp_figure.dots[i].x >= 0) //Проверяем только те точки на принадлежность к руинам, которые находятся внутри расчетной области
    {
      if (ruins[temp_figure.dots[i].x][temp_figure.dots[i].y] == true)
      {
        no_intersection = false;
      }      
    }
  }
  if (no_intersection)
  {
    act_figure = temp_figure;
  }
}

///Функция проверки пересечения фигурой развалин следующим шагом
bool intersection(byte slide_direction)
{
  switch (slide_direction)
  {
    case left:
      {
        for (byte i = 0; i < 4; i++)
        {
          if ((act_figure.dots[i].y - 1 < 0))
          {
            return true;
          } else if ((act_figure.dots[i].x >= 0) && (act_figure.dots[i].x < 8)
                     && (act_figure.dots[i].y > 0) && (act_figure.dots[i].y < 8)) //Ограничиваемся по размеру поля минус колонка слева
          {
            if (ruins[act_figure.dots[i].x][act_figure.dots[i].y - 1] == true)
            {
              return true;
            }
          }
        }
        return false;
        break;
      }
    case right:
      {
        for (byte i = 0; i < 4; i++)
        {
          if ((act_figure.dots[i].y + 1 > 7))
          {
            return true;
          } else if ((act_figure.dots[i].x >= 0) && (act_figure.dots[i].x < 8)
                     && (act_figure.dots[i].y >= 0) && (act_figure.dots[i].y < 8)) //Ограничиваемся по размеру поля минус колонка справа
          {
            if (ruins[act_figure.dots[i].x][act_figure.dots[i].y + 1] == true)
            {
              return true;
            }
          }
        }
        return false;
        break;
      }
    case down:
      {
        for (byte i = 0; i < 4; i++)
        {
          if (act_figure.dots[i].x + 1 > 7)
          {
            return true;
          } else if (act_figure.dots[i].x + 1 >= 0)
          {
            if (ruins[act_figure.dots[i].x + 1][act_figure.dots[i].y] == true)
            {
              return true;
            }
          }
        }
        return false;
        break;
      }
  }
}

///Функция, определяющая какой ряд заполнен и выдающая это в формате B01101100
byte full_rows()
{
  byte answer = B00000000;
  for (byte i = 0; i < 8; i++)
  {
    byte sum = 0;
    for (byte j = 0; j < 8; j++)
    {
      //Serial.print(ruins[i][j]);
      //Serial.print(", ");
      sum += ruins[i][j];
    }
    //Если все точки в ряду закрашены, то устанавливаем единицу в этот ряд
    if (sum == 8)
    {
      answer += round(pow(2, 7 - i));
    }
  }
  return answer;
}

///Функция добавления текущего фрагмента к развалинам
void ruins_add_fr()
{
  bool you_lost = false;

  for (byte i = 0; i < 4; i++)
  {
    if (act_figure.dots[i].x < 0)
    {
      you_lost = true;
    }
  }
  if (you_lost)
  {
    game_over();
  } else
  {
    for (byte i = 0; i < 4; i++)
    {
      ruins[act_figure.dots[i].x][act_figure.dots[i].y] = true;
    }
  }
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

///Функция, обнуляющая руины
void null_ruins()
{
  for (byte i = 0; i < 8; i++)
  {
    for (byte j = 0; j < 8; j++)
    {
      ruins[i][j] = 0;
    }
  }
}

///Функция, раскидывающая на поле две рандомные фигуры тетрамино. Используется при генерации нового поля
void generate_ruins()
{
  new_figure();
  for (byte i = 0; i < 4; i++)
  {
    step_left();
    actualize_pole();
    display_pole();
  }
  for (byte i = 0; i < 9; i++)
  {
    step_down();
    actualize_pole();
    display_pole();
  }
  new_figure();
  for (byte i = 0; i < random(2, 4); i++)
  {
    step_right();
    actualize_pole();
    display_pole();
  }
  for (byte i = 0; i < 9; i++)
  {
    step_down();
    actualize_pole();
    display_pole();
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
  null_ruins();
  generate_ruins();
  new_figure();
  actualize_pole();
  while (digitalRead(start_pause_button_Pin) == LOW)
  {
    display_pole();
  }
}
