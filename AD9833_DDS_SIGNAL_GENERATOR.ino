#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "AD9833.h"

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

AD9833 AD(D8); // dataPin, clkPin, fsyncPin or fsyncPin only

char wave[5][16] = {"OFF", "SINE" , "SQUARE1" , "SQUARE2" ,"TRIANGLE"};
int wave_count = 5;
int wave_idx = 0; // default OFF 
int last_wave_idx = 0;

uint8_t  ch = 0; // 0 or 1, default 0

bool last_state_1 = 0;
unsigned long int counter = 1;
unsigned long int last_counter = 1;

unsigned long int last_isr_millis = 0;
int debounce_interval = 500;

void setup()
{
  Serial.begin(115200);

	// initialize the LCD
	lcd.begin();
	
  // Turn on the blacklight and print a message.
	lcd.backlight();
  lcd.setCursor(0, 0);
	lcd.print("   AD9833 DDS   ");
  lcd.setCursor(0, 1);
  lcd.print("Signal Generator");

  SPI.begin();
  AD.begin();
  AD.setFrequency(0, ch);   // (freq, channel) --> 1000 Hz.
  AD.setWave(wave_idx);

  delay(5000);
  
  // set default display
  drawFrequency(0);
  drawWave();

  // rotary encode button as pin interrupt
  attachInterrupt(digitalPinToInterrupt(D6), button_handler, FALLING);
  pinMode(D6, INPUT_PULLUP);

  // rotary encoder pin
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
}


void loop()
{
  setFrequency();
  setWave();
  delay(1);
  
}

void drawFrequency(float currFrequency)
{
  clearRow(0);
  lcd.setCursor(0, 0);
  lcd.print("Freq : ");
  Serial.print(counter);
  Serial.print(" : ");

  if(currFrequency >= 1000000L){
    lcd.print(currFrequency/1000000L);
    lcd.print(" MHz");
    Serial.print(currFrequency/1000000L);
    Serial.println(" MHz)");
  }
  else {
    lcd.print(currFrequency/1000);
    lcd.print(" KHz");
    Serial.print(currFrequency/1000);
    Serial.println(" KHz)");
  }
}

void setFrequency(){
  bool state_1 = digitalRead(D4);
  if(last_state_1 != state_1 && state_1 == 1) {
    if(digitalRead(D3) != state_1) {
      counter = counter + 1;
    }
    else {
      counter = counter - 1;
    }
  }
  last_state_1 = state_1;

   if(counter <= 0) {
    counter = 1; 
   }
   if(counter >= 1000000) {
    counter = 1000000;
   }
  if(last_counter != counter) {
    AD.setFrequency(counter * 1000, ch);
    last_counter = counter;
    drawFrequency(AD.getFrequency());
  }
}

void drawWave(){
  clearRow(1);
  lcd.setCursor(0, 1);
  lcd.print("Wave : ");
  lcd.print(wave[wave_idx]);
  Serial.print("Wave : ");
  Serial.println(wave[wave_idx]);
}


void setWave(){
  if(last_wave_idx != wave_idx){
    AD.setWave(wave_idx);
    last_wave_idx = wave_idx;
    drawWave();
  }
}

void clearRow(int row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
}

ICACHE_RAM_ATTR void button_handler(){
  if(millis() - last_isr_millis >= debounce_interval) {
    wave_idx++;
    
    if(wave_idx >= wave_count) {
      wave_idx = 0;
    }

    last_isr_millis = millis();
  }
}