#include <Countimer.h>
#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Setting up some stuff
Countimer timer;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //change I2C address for your lcd

unsigned long prevTime = millis();
unsigned long prevTime2 = millis();
unsigned long prevTime3 = millis();
unsigned long prevTime4 = millis();
unsigned long prevTime5 = millis();

int enabl;
int enabl2;
int enabl3;
int enabl4;
int enabl5;
int enabl6;
int enabl7;
int enabl8;
int enabl9;
int enabl10;
int enabl11;
int enabl12 = 0;
int enabl13;
int errcode = 3; // change count of attempts 
String code;
String truecode;
String sektime;
String mintime;

char key;
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// keypad L-to-R : 8,7,6,5,4,3,2
byte rowPins[ROWS] = { 8, 7, 6, 5 };
byte colPins[COLS] = { 4, 3, 2 }; 

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup()
{
  // Initialize some stuff
  lcd.begin(20, 4);
  lcd.backlight();
  pinMode(9, INPUT_PULLUP); //wires for explosive
  pinMode(14, INPUT_PULLUP); //button for opening
  pinMode(10, OUTPUT); //relay for fire
  pinMode(11, INPUT_PULLUP); //key switch
  digitalWrite(10, LOW);
  // 12 - piezo 1
  // 13 - piezo 2
  // Test piezos
  tone(12, 2000, 300);
  delay(400);
  tone(13, 2000, 300);
  //Set code
  lcd.setCursor(2,1);
  lcd.print("Enter code:");
  while (enabl4 != 1) {
    key = kpd.getKey();
    if (key) {
      truecode += key;
      lcd.setCursor(2,1);
      lcd.print("Enter code: " + truecode);
    }
    if (truecode.length() > 3) {
      enabl4 = 1;
      tone(13, 5000, 100);
      delay(200);
    }
  }
  // Set time
  lcd.setCursor(0,2);
  lcd.print("Enter time-h:");
  while (enabl6 != 1) {
    key = kpd.getKey();
    if (key && key != '*' && key != '#') {
      sektime += key;
      lcd.setCursor(0,2);
      lcd.print("Enter time-h: " + sektime);
    } else if(key == '#') {
      enabl6 = 1;
      tone(13, 5000, 100);
      delay(200);
    }
  }
  lcd.setCursor(0,3);
  lcd.print("Enter time-m:");
  while (enabl13 != 1) {
    key = kpd.getKey();
    if (key) {
      mintime += key;
      lcd.setCursor(0,3);
      switch (key) {
        case '#':
          enabl13 = 1;
          tone(13, 5000, 100);
          delay(500);
          break;
      }
      lcd.print("Enter time-m: " + mintime);
    }
  }
  // Initialize timer
  int intsektime = sektime.toInt();
  int intmintime = mintime.toInt();
  timer.setCounter(intsektime, intmintime, 0, timer.COUNT_DOWN, onComplete);
  timer.setInterval(refreshClock, 1000);
  //Clear lcd and write some stuff
  lcd.clear();
  lcd.setCursor(0,2);
  lcd.print("Bomb has been armed!");
}

void(* resetFunc) (void) = 0;

void loop() 
{
  // Start timer
  timer.run();
  if (!timer.isCounterCompleted()) {
    timer.start();
  }
  // Get key from keypad
  key = kpd.getKey();
  // Entering code
  if(enabl11 !=1) {
    lcd.setCursor(2,3);
    lcd.print("Enter code: ");
    enabl11 = 1;
  }
  if ((key) && (enabl2 != 1)) {
    tone(12, 1500, 200);
    code += key;
    lcd.setCursor(2,3);
    lcd.print("Enter code: " + code);
  }
  // Defused using code or true code
  if(code.length() > 3) {
    if (code == truecode) {
      timer.stop();
      delay(400);
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("   Bomb has been    ");
      lcd.setCursor(0,3);
      lcd.print("      DEFUSED       ");
      code = "";
      enabl = 1;
      delay(200);
      tone(12, 2500, 100);
      delay(200);
      tone(12, 2500, 100);
      while (enabl5 != 1) {
        if (digitalRead(11) == LOW) {
          delay(1000);
          resetFunc();
        }
      }
     } else {
      delay(400);
      errcode = errcode - 1;
      if(errcode == 0) {
        timer.stop();
        onComplete();
        enabl12 = 1;
      }
      tone(12, 800, 300);
      lcd.setCursor(2,3);
      lcd.print("Enter code:     ");
      code = "";
     }
  }
  // Defused using key
  if (digitalRead(11) == LOW) { 
    lcd.setCursor(0,1);
    lcd.print("                    ");
    lcd.setCursor(0,2);
    lcd.print("   Bomb has been    ");
    lcd.setCursor(0,3);
    lcd.print("      DEFUSED       ");
    timer.stop();
    code = "";
    enabl = 1;
    delay(200);
    tone(12, 2500, 100);
    delay(200);
    tone(12, 2500, 100);
    delay(2000);
    while (enabl5 != 1) {
      if (digitalRead(11) == LOW) {
        delay(1000);
        resetFunc();
      }
    }
  }
  // If wire cut to the explosive, allahu akbar
  if(enabl12 !=1) {
    if(digitalRead(9) == HIGH) {
      timer.stop();
      onComplete();
      enabl12 = 1;
    }
  }
  // If bomb opened, allahu akbar
  if(enabl12 !=1) {
    if(digitalRead(14) == HIGH) {
      timer.stop();
      onComplete();
      enabl12 = 1;
    }
  }
  // Just beeping before allahu akbar
  if ((millis() - prevTime >= 2000) && (enabl != 1)) {
    prevTime = millis();
    if (enabl7 != 1) {
      tone(12, 2200, 200);
    }
  }
  if ((millis() - prevTime2 >= 1000) && (enabl != 1)) {
    prevTime2 = millis();
    if ((enabl8 != 1) && (timer.getCurrentSeconds() < 20) && (timer.getCurrentMinutes() == 0) && (timer.getCurrentHours() == 0)) {
      tone(12, 2200, 200);
      enabl7 = 1;
    }
  }
  if ((millis() - prevTime3 >= 500) && (enabl != 1)) {
    prevTime3 = millis();
    if ((enabl9 != 1) && (timer.getCurrentSeconds() < 10) && (timer.getCurrentMinutes() == 0) && (timer.getCurrentHours() == 0)) {
      tone(12, 2200, 200);
      enabl8 = 1;
    }
  }
  if ((millis() - prevTime4 >= 150) && (enabl != 1)) {
    prevTime4 = millis();
    if ((enabl10 != 1) && (timer.getCurrentSeconds() < 5) && (timer.getCurrentMinutes() == 0) && (timer.getCurrentHours() == 0)) {
      tone(12, 2200, 100);
      enabl9 = 1;
    }
  }
  if ((millis() - prevTime5 >= 100) && (enabl != 1)) {
    prevTime5 = millis();
    if ((timer.getCurrentSeconds() < 2) && (timer.getCurrentMinutes() == 0) && (timer.getCurrentHours() == 0)) {
      tone(12, 2200, 50);
      enabl10 = 1;
    }
  }
}

void refreshClock() {
  // Show time left
  lcd.setCursor(6,0);
  lcd.print(timer.getCurrentTime());
}

void onComplete() {
  //Print some stuff after allahu akbar
  lcd.setCursor(6,0);
  lcd.print("00:00:00");
  lcd.setCursor(0,1);
  lcd.print("                    ");
  lcd.setCursor(0,2);
  lcd.print(" Bomb has exploded!  ");
  lcd.setCursor(0,3);
  lcd.print("Turn key to restart!");
  enabl2 = 1;
  enabl = 1;
  // Turn on relay and generate tone
  digitalWrite(10, HIGH);
  tone(12, 400, 2000);
  delay(2000);
  digitalWrite(10, LOW);
  // Just wait for the restart using key
  while (enabl3 != 1) {
    if (digitalRead(11) == LOW) {
      delay(1000);
      resetFunc();
    }
  }
}
