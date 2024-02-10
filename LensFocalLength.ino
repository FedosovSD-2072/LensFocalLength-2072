// ППО-2024, устройство определения фокусного расстояния линзы, Школа №2072 //

#define DEBUG1 1  // отладка вкл/выкл

#define POTPIN A5           // потенциометр: пин
#define POTMIN 2            // потенциометр: минимальное значение
#define POTMAX 101          // потенциометр: максимальное значение
#define LIGHTPIN A4         // фоторезистор: пин
#define LIGHTSENSORDIST 15  // потенциометр: поправочный коэффециент расстояния до линзы
#define SERVOPIN 9          // сервопривод: пин
#define SERVOSPEED -90      // сервопривод: скорость
#define BUTTONPIN 10        // кнопка: пин

// Задаём пины дисплея:
#define PIN_RS 2
#define PIN_EN 3
#define PIN_DB4 5
#define PIN_DB5 6
#define PIN_DB6 7
#define PIN_DB7 8

// подключаем библиотеки (БИБЛИОТЕКИ ДЛЯ РАБОТЫ С ДАТЧИКАМИ НЕ ИСПОЛЬЗУЮТСЯ):
#include <LiquidCrystal.h>                                              // Библиотека для работы с текстовым дисплеем
LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_DB4, PIN_DB5, PIN_DB6, PIN_DB7);  // создание дисплея
#include <Servo.h>                                                      // библиотека серво
Servo myservo;                                                          //создаём серво

int potval, potval_max, lightval, lightval_max;  // значения потенциометра и фоторезистора
int currLength, focalLength;                     // переменные для хранения длин

void setup() {
  lcd.begin(16, 2);                   // Устанавливаем размер экрана: количество столбцов и строк
  lcd.setCursor(0, 0);                // Устанавливаем курсор в колонку 0 и строку 0
  lcd.print("Hello               ");  // текст на экране

  //режим работы пинов:
  pinMode(POTPIN, INPUT);
  pinMode(LIGHTPIN, INPUT);
  pinMode(BUTTONPIN, INPUT_PULLUP);

  //СОМ-порт:
  Serial.begin(9600);
  Serial.print("\n\n\n");

  /* подключение и тест сервопривода: */ {
    myservo.attach(SERVOPIN);
    myservo.write(90);
    delay(1000);
    myservo.write(90 - SERVOSPEED);
    Serial.println("Servo testing >>>");
    delay(2000);
    myservo.write(90 + SERVOSPEED);
    Serial.println("Servo testing <<<");
    delay(2000);
    myservo.write(90);
  }
  Serial.println("Done");

  lcd.setCursor(0, 0);                    // Устанавливаем курсор в колонку 0 и строку 0
  lcd.print("Click the button        ");  // текст на экране
}

void loop() {
  lightval = map(analogRead(LIGHTPIN), 0, 1023, 1023, 0);  // читаем данные с фоторезистора

  if (!digitalRead(BUTTONPIN)) {           // если кнопка нажата
    lcd.setCursor(0, 0);                   // Устанавливаем курсор в колонку 0 и строку 0
    lcd.print("Please wait...         ");  // текст на экране

    // обнуляем переменные и ждем:
    lightval = 0;
    potval = 0;
    lightval_max = 0;
    potval_max = 0;
    delay(300);

    potval = 0.1 * analogRead(POTPIN);  // читаем значение с потенциометра и делим на 10, чтобы привести значения к одному порядку

    // движемся вперед, пока не упремся в минимум, и считываем данные:
    while (potval > POTMIN) {
      myservo.write(90 - SERVOSPEED);
      delay(20);
      LightMAX_func();    // ищем максимум яркости
      Debug1();           // отладка
      CurrLengthCalc();   // вычисляем текущее расстояние
      FocalLengthCalc();  // вычисляем фокусное расстояние
      Serial.println();
      delay(30);
    }
    delay(100);
    // движемся назад, пока не упремся в максимум, и считываем данные:
    while (potval < POTMAX) {
      myservo.write(90 + SERVOSPEED);
      delay(20);
      LightMAX_func();    // ищем максимум яркости
      Debug1();           // отладка
      CurrLengthCalc();   // вычисляем текущее расстояние
      FocalLengthCalc();  // вычисляем фокусное расстояние
      Serial.println();
      delay(30);
    }
    delay(100);
    // движемся вперед, пока недоедем до полученного значения фокусного расстояния:
    while (potval > potval_max) {
      potval = 0.1 * analogRead(POTPIN);
      myservo.write(90 - SERVOSPEED);
    }

    FocalLengthCalc();  // вычисляем фокусное расстояние

    lcd.setCursor(0, 0);                 // Устанавливаем курсор в колонку 0 и строку 0
    lcd.print("Focal Length         ");  // текст на экране
    lcd.setCursor(0, 1);                 // Устанавливаем курсор в колонку 0 и строку 1
    lcd.print(focalLength);              // выводим на экран фокусное расстояние
    lcd.print(" mm              ");      // текст на экране
  }
  myservo.write(90);  // останавливаем сервопривод
  Debug1();           // отладка, если включена
  delay(50);          // ждём
}

//отладка:
void Debug1() {
  if (DEBUG1) {
    Serial.print("lightval = " + (String)lightval + "  ");
    Serial.print("lightval_max = " + (String)lightval_max + "  ");
    Serial.print("potval = " + (String)potval + "  ");
    Serial.print("potvalmax = " + (String)potval_max + "  ");
  }
}

// вычисляем текущее расстояние:
void CurrLengthCalc() {
  if (potval <= 86) currLength = 0.4 * potval + LIGHTSENSORDIST;  // если значение потенциометра меньше 87, то используемпервую формулу
  else currLength = 3.75 * potval - 287.5 + LIGHTSENSORDIST;      // иначе - вторую
  Serial.print("   CL = ");
  Serial.print(currLength);

  lcd.setCursor(0, 0);                     // Устанавливаем курсор в колонку 0 и строку 0
  lcd.print("Current Position         ");  // текст на экране
  lcd.setCursor(0, 1);                     // Устанавливаем курсор в колонку 0 и строку 1
  lcd.print(currLength);                   // выводим на экран ntreott расстояние
  lcd.print(" mm              ");          // текст на экране
}

// вычисляем фокусное расстояние:
void FocalLengthCalc() {
  if (potval_max <= 86) focalLength = 0.4 * potval_max + LIGHTSENSORDIST;  // если значение потенциометра меньше 87, то используемпервую формулу
  else focalLength = 3.75 * potval_max - 287.5 + LIGHTSENSORDIST;          // иначе - вторую
  Serial.print("   FL = ");
  Serial.print(focalLength);
}

// ищем максимум яркости:
void LightMAX_func() {
  lightval = map(analogRead(LIGHTPIN), 0, 1023, 1023, 0);  // читаем значение с фоторезистора и инвертируем значение
  potval = 0.1 * analogRead(POTPIN);                       // читаем значение с потенциометра и делим на 10, чтобы привести значения к одному порядку
  if (lightval > lightval_max) {                           // ищем максимумы
    lightval_max = lightval;
    potval_max = potval;
  }
}
