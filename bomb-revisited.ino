#include <Countimer.h>
#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Setting up some stuff
Countimer timer;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //change I2C address for your lcd

unsigned long prevTime = millis();
bool codeSet = false, timeSetHour = false, timeSetMinute = false;
String code, truecode, minSet, hourSet;
int errcode = 3; // change count of attempts 

char key;
char keys[4][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// keypad L-to-R : 8,7,6,5,4,3,2
byte rowPins[4] = { 8, 7, 6, 5 };
byte colPins[3] = { 4, 3, 2 }; 

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, 4, 3 );

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
  while (!codeSet) {
    key = kpd.getKey();
    if (key && key != '*' && key != '#') {
      truecode += key;
      lcd.setCursor(2,1);
      lcd.print("Enter code: " + truecode);
    }
    if (truecode.length() > 3) {
      codeSet = true;
      tone(13, 5000, 100);
      delay(200);
    }
  }
  // Set time
  lcd.setCursor(0,2);
  lcd.print("Enter time-h:");
  while (!timeSetHour) {
    key = kpd.getKey();
    if (key && key != '*' && key != '#') {
      hourSet += key;
      lcd.setCursor(0,2);
      lcd.print("Enter time-h: " + hourSet);
    } else if(key == '#') {
      timeSetHour = true;
      tone(13, 5000, 100);
      delay(200);
    }
  }
  lcd.setCursor(0,3);
  lcd.print("Enter time-m:");
  while (!timeSetMinute) {
    key = kpd.getKey();
    if (key && key != '*' && key != '#') {
      minSet += key;
      lcd.setCursor(0,3);
      lcd.print("Enter time-m: " + minSet);
    } else if (key == '#') {
	  timeSetMinute = true;
      tone(13, 5000, 100);
      delay(500);
	}
  }
  // Initialize timer
  timer.setCounter(hourSet.toInt(), minSet.toInt(), 0, timer.COUNT_DOWN, onComplete);
  timer.setInterval(refreshClock, 1000);
  //Clear lcd and write some stuff
  lcd.clear();
  lcd.setCursor(0,2);
  lcd.print("Bomb has been armed!");
  lcd.setCursor(2,3);
  lcd.print("Enter code: ");
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
  if (key) {
    tone(12, 1500, 200);
    code += key;
    lcd.setCursor(2,3);
    lcd.print("Enter code: " + code);
  }
  
  // Defused using code or true code
  if(code.length() > 3) {
    if (code == truecode) {
      bombDefused();
    } else {
      delay(400);
      errcode = errcode - 1;
      if(errcode == 0) {
        timer.stop();
        onComplete();
      }
      tone(12, 800, 300);
      lcd.setCursor(2,3);
      lcd.print("Enter code:     ");
      code = "";
     }
  }
  // Defused using key
  if (digitalRead(11) == LOW) { 
    bombDefused();
  }
  // If wire cut to the explosive, allahu akbar
  if(digitalRead(9) == HIGH) {
    timer.stop();
    onComplete();
  }
  // If bomb opened, allahu akbar
  if(digitalRead(14) == HIGH) {
    timer.stop();
    onComplete();
  }
  
  // Just beeping before allahu akbar
  if ((millis() - prevTime >= 100) && (timer.getCurrentSeconds() < 2) && (timer.getCurrentMinutes() == 0) && (timer.getCurrentHours() == 0)) {
    prevTime = millis();
    tone(12, 2200, 50);
  } else if ((millis() - prevTime >= 150) && (timer.getCurrentSeconds() < 5) && (timer.getCurrentMinutes() == 0) && (timer.getCurrentHours() == 0)) {
	prevTime = millis();
    tone(12, 2200, 100);
  } else if ((millis() - prevTime >= 500) && (timer.getCurrentSeconds() < 10) && (timer.getCurrentMinutes() == 0) && (timer.getCurrentHours() == 0)) {
	prevTime = millis();
    tone(12, 2200, 200);
  } else if ((millis() - prevTime >= 1000) && (timer.getCurrentSeconds() < 20) && (timer.getCurrentMinutes() == 0) && (timer.getCurrentHours() == 0)) {
	prevTime = millis();
    tone(12, 2200, 200);
  } else if (millis() - prevTime >= 2000) {
	prevTime = millis();
	tone(12, 2200, 200);
  }
}

void bombDefused() {
  timer.stop();
  lcd.setCursor(0,1);
  lcd.print("                    ");
  lcd.setCursor(0,2);
  lcd.print("   Bomb has been    ");
  lcd.setCursor(0,3);
  lcd.print("      DEFUSED       ");
  code = "";
  delay(200);
  tone(12, 2500, 100);
  delay(200);
  tone(12, 2500, 100);
  delay(2000); //delay for key
  while (true) {
    if (digitalRead(11) == LOW) {
      delay(1000);
      resetFunc();
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
  // Turn on relay and generate tone
  digitalWrite(10, HIGH);
  tone(12, 400, 2000);
  delay(2000);
  digitalWrite(10, LOW);
  // Just wait for the restart using key
  while (true) {
    if (digitalRead(11) == LOW) {
      delay(1000);
      resetFunc();
    }
  }
}
