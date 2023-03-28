// 
// 7-segment indicator Demo
//
// Hardware:
// - Arduino Nano
// - 74HC595N shift register
// - 7-segment indicator with any number of digits. Common Cathode
// - 220R resistors
// 

// Shift register pins
const byte clockPin = 4;
const byte latchPin = 5;
const byte dataPin = 6;

// 7-segment indicator settings
const byte digitsCount = 3;
const byte digitPins[] = {8, 9, 10}; //digit cathode pin

//switch to 1 if you want to debug via serial monitor
const bool debugMode = 0;

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

word counter = 255;
const word stepDuration = 1000;

const int maxNumber = pow(10, digitsCount) - 1;  

void setup() {
  initializePins();
  turnOffDisplay();
  displayNumber(emptyDigit);
  delay(100);
  if (debugMode) {
    Serial.begin(9600);    
  }
}

void loop() {
  unsigned long timeForNextStep = millis() + stepDuration;
  while (millis() < timeForNextStep) {
    displayNumber(counter);
  }
  counter--;
}

void displayNumber(int number) {
  number = constrain(number, 0, maxNumber);

  byte numbers[digitsCount];
  for(byte i = 0; i < digitsCount; i++) {
    numbers[i] = extractNumber(number, i);  
  }

  debugNumbers(numbers);
  
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
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, digitByte);
  digitalWrite(latchPin, HIGH);
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