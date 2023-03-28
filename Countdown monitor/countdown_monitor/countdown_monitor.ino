// 
// Countdown monitor
//
// Hardware:
// - Arduino Nano
// - DS3231 Time Module
// - 74HC595N shift register
// - 5361AS 7-segment indicator with 3 digits
// - 220R resistors
//
// Arduino interact with time module via I2C.
// 

#include <TimeLib.h>
#include <Wire.h> 
#include <DS1307RTC.h>

// Shift register pins
const byte clockPin = 4;
const byte latchPin = 5;
const byte dataPin = 6;

// 7-segment indicator settings
const byte digitsCount = 3;
const byte digitPins[] = {8, 9, 10}; //digit cathode pin

//switch to 1 if you want to debug via serial monitor
const bool debugMode = 1;

const time_t dayOfFreedom = 1692795600; //2023-08-23

//    A
//  XXXXX
//F X   X B
//  XXXXX  G
//E X   X C
//  XXXXX  X H
//    D
const byte digitsMap[10] = {
// ABCDEFGH  
  B11111100, //0
  B01100000, //1
  B11011010, //2
  B11110010, //3
  B01100110, //4
  B10110110, //5
  B10111110, //6
  B11100000, //7
  B11111110, //8
  B11110110  //9
};
const byte emptyDigit = B00000000;

word stepDuration = 60 * 1000;  // once per minute
const int maxNumber = pow(10, digitsCount) - 1;  

void setup() {
  initializePins();
  turnOffDisplay();
  displayNumber(emptyDigit);
  setSyncProvider(RTC.get);
  delay(100);
  if (debugMode) {
    Serial.begin(9600);
    Serial.println("Application started"); 
  }
}

void loop() {
  time_t timeNow = now(); 
  int daysToFreedom = elapsedDays(dayOfFreedom) - elapsedDays(timeNow);

  unsigned long timeForNextStep = millis() + stepDuration;
  while (millis() < timeForNextStep) {
    displayNumber(daysToFreedom);
  }
  if (debugMode) {
    Serial.print("Days to freedom: ");
    Serial.print(daysToFreedom);
    Serial.println();
  }
}

void displayNumber(int number) {
  number = constrain(number, 0, maxNumber);

  byte numbers[digitsCount];
  for(byte i = 0; i < digitsCount; i++) {
    numbers[i] = extractNumber(number, i);  
  }

  // debugNumbers(numbers);
  
  byte digits[digitsCount];
  bool previousIsNull = 1;
  for(byte i = 0; i < digitsCount; i++) {
    if (previousIsNull) {
      bool isLastDigit = i == (digitsCount - 1);
      if (isLastDigit) {
        //want to show zero
        digits[i] = mapDigitToByte(numbers[i]);
      } else {
        digits[i] = mapDigitToByteOrEmpty(numbers[i]);        
      }
    } else {
      digits[i] = mapDigitToByte(numbers[i]);      
    }
    if (numbers[i] != 0) {
      previousIsNull = 0;
    }
  }

  displayDigits(digits);
}

byte extractNumber(int number, byte position) {
  return (number / intPow(10, digitsCount - position - 1)) % 10;
}

void displayDigits(byte digits[]) {
  for (byte i = 0; i < digitsCount; i++) {
    displayDigit(digits[i], i);
  }
}

// Display one digit on specified seveg-segment display
void displayDigit(byte digit, byte digitIndex) {
  displayDigit(emptyDigit);
  digitalWrite(digitPins[digitIndex], LOW);
  displayDigit(digit);
  delay(1);
  digitalWrite(digitPins[digitIndex], HIGH);
}

void displayDigit(byte digitByte) {
  digitalWrite(latchPin, LOW); //Открытие защелки
  shiftOut(dataPin, clockPin, LSBFIRST, digitByte);
  digitalWrite(latchPin, HIGH); //Закрытие защелки
}

byte mapDigitToByte(byte digit) {
  return digitsMap[digit % 10];
}

byte mapDigitToByteOrEmpty(byte digit) {
  if (digit == 0) {
    return emptyDigit;
  } else {
    return mapDigitToByte(digit);
  }
}

void initializePins() {
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  for (byte i = 0; i < digitsCount; i++) {
    pinMode(digitPins[i], OUTPUT);
  }
}

void turnOffDisplay() {
  for (byte i = 0; i < digitsCount; i++) {
    digitalWrite(digitPins[i], HIGH);
  }
}

//stolen from StackOverflow https://stackoverflow.com/questions/101439
int intPow(int base, int exp) {
  int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

// ##########################
// ONLY FOR DEBUGGING
// ##########################

void debugNumbers(byte numbers[]) {
  if (debugMode) {
    for (byte i = 0; i < digitsCount; i++) {
      Serial.print(numbers[i]);
      Serial.write(' ');
    }
    Serial.println();  
  }
}

void printDateTime(TimeElements dateTime) {
  if (debugMode) {
    Serial.print(tmYearToCalendar(dateTime.Year));
    Serial.write('-');
    print2digits(dateTime.Month);
    Serial.write('-');
    print2digits(dateTime.Day);
    Serial.write(' ');
    print2digits(dateTime.Hour);
    Serial.write(':');
    print2digits(dateTime.Minute);
    Serial.write(':');
    print2digits(dateTime.Second);
    Serial.println();
  }
}

void print2digits(int number) {
  if (debugMode) {
    if (number >= 0 && number < 10) {
      Serial.write('0');
    }
    Serial.print(number);
  }
}