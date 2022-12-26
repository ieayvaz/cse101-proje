#include <MD_MAX72xx.h>
#include <LiquidCrystal_I2C.h>
#include "math.h"
#include "pitches.h"

#define	MAX_DEVICES	2

#define SPEAKER_PIN 12

const int maxX = MAX_DEVICES * 8 - 1;
const int maxY = 7;

#define	CLK_PIN		13
#define	DATA_PIN	11
#define	CS_PIN		10

#define VERT_PIN A0
#define HORZ_PIN A1
#define SEL_PIN  2

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::PAROLA_HW, CS_PIN, MAX_DEVICES);

#define I2C_ADDR    0x27
#define LCD_COLUMNS 16
#define LCD_LINES   2

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

enum states{
  INTRO,PIXEL_SELECTION,FIND_PIXEL,GAMEOVER
};
states flag = INTRO;
int x = 0;
int y = 0;
int initial = 0;
int final = 0;
int randx = 0;
int randy = 0;
int selected_x = 0;
int selected_y = 0;
int a = 0;
int first_x = 0;
int first_y = 0;

//sıcak esik degeri, bu mesafenin altı sıcak üstü soguk olur
int hotthreshold = 14;

//en yuksek mesafe 
int biggestdist = pow(maxX,2) + pow(maxY,2);


void pixelMotion(){
  int horz = analogRead(HORZ_PIN);
  int vert = analogRead(VERT_PIN);
  if (vert < 300) {
    y = min(y + 1, maxY);
    tone(SPEAKER_PIN, NOTE_DS5);
    delay(50);
    noTone(SPEAKER_PIN);
  }
  if (vert > 700) {
    y = max(y - 1, 0);
    tone(SPEAKER_PIN, NOTE_DS5);
    delay(50);
    noTone(SPEAKER_PIN);
  }
  if (horz > 700) {
    x = min(x + 1, maxX);
    tone(SPEAKER_PIN, NOTE_DS5);
    delay(50);
    noTone(SPEAKER_PIN);
  }
  if (horz < 300) {
    x = max(x - 1, 0);
    tone(SPEAKER_PIN, NOTE_DS5);
    delay(50);
    noTone(SPEAKER_PIN);
  }
  mx.clear();
  mx.setPoint(y, x, true);
  mx.update();
  delay(50);
}

void randomPosition() {
  if (a == 0) {
    x = random(maxX + 1);
    y = random(maxY + 1);
    first_x = x;
    first_y = y;
    mx.clear();
    mx.setPoint(y, x, true);
    mx.update();
    a = 1;
  }
}

void coldSound(int level){
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("cold");
  tone(SPEAKER_PIN, NOTE_DS5);
  delay(100 + level);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(100 + level);
  tone(SPEAKER_PIN, NOTE_CS5);
  delay(100 + level);
  for (byte i = 0; i < 10; i++) {
    for (int pitch = -10; pitch <= 10; pitch++) {
      tone(SPEAKER_PIN, NOTE_C5 + pitch);
      delay(5);
    }
  }
  noTone(SPEAKER_PIN);
  lcd.clear();
}

void hotSound(int level){
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("hot");
  tone(SPEAKER_PIN, NOTE_E4 + level);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G4 + level);
  delay(150);
  tone(SPEAKER_PIN, NOTE_E5 + level);
  delay(150);
  tone(SPEAKER_PIN, NOTE_C5 + level);
  delay(150);
  tone(SPEAKER_PIN, NOTE_D5 + level);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G5 + level);
  delay(150);
  noTone(SPEAKER_PIN);
  lcd.clear();
}

void intro() {
  if (flag == 0) {
    lcd.setCursor(3, 0);
    lcd.print("welcome to");
    lcd.setCursor(4, 1);
    lcd.print("the game");
    delay(3000);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("select a pixel");
    flag = 1; 
  }
}

void pixelSelection(){
  if (flag == 1) {
    pixelMotion();
    if (digitalRead(SEL_PIN) == LOW) {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("pixel selected");
      lcd.setCursor(4, 1);
      lcd.print("and sent");
      Serial.print("seciminiz : ");
      Serial.print(x);
      Serial.print(", ");
      Serial.println(y);
      delay(1000);
      lcd.setCursor(0, 0);
      lcd.clear();
      lcd.print("make your guess!");
      flag = 2;
    }
  }
}

void findPixel() {
  if (flag == 2) {
    initial = pow(first_x - randx, 2) + pow(first_y - randy, 2);
    randomPosition();
    pixelMotion();
    if (digitalRead(SEL_PIN) == LOW) {
      first_x = x;
      first_y = y;
      final = pow(x - randx, 2) + pow(y - randy, 2);
      if(x == randx && y == randy){
        flag = 3;
      }

      int hotlevel = 1500 - map(final,0,hotthreshold,0,1500);
      int coldlevel = map(final,hotthreshold,biggestdist/2,0,500);

      if(x == randx && y == randy){
        flag = 3;
      }
      else if(final > hotthreshold){
        coldSound(coldlevel);
      }
      else{
        hotSound(hotlevel);
      }
    }
  }
}

void gameOver(){
  if (flag == 3) {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("you win");
    flag = 4;
  }
}

void setup() {
  randomSeed(analogRead(A3));
  Serial.begin(9600);
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);
  mx.clear();

  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
  pinMode(SEL_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  randx = random(maxX + 1);
  randy = random(maxY + 1);

  x = random(maxX + 1);
  y = random(maxY + 1);
}

void loop() {
  intro();
  pixelSelection();
  findPixel();
  gameOver();
}
