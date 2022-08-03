#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal.h>

// Code based on arduino LCD example

// To generate a code, the program generates a procedural random sequence of numbers based on the date which it matched to the keypad
// To ensure it does not exceed its memory limit, the numbers in the procedural generation sequence are used as checks multiple times
// Valid codes reset every hour, but give 5334118 a try...

#define BUTTON_PIN 14

#define OVERFLOW 840 // How many codes to present before repeating in an hour. With 840 there is a 0.09% chance they guess a correct code with 899999 possibillities. Must be the same as on the keypad 

// Wifi can not have a captive gateway
const char* ssid = ""; // Replace with your WIFI SSID
const char* password = ""; // Your WIFI Password

int timeHour = 0;
int timeMinute = 0;
int timeSecond = 0;

int buttonPresses = 0;

int timeUntilReset = 0;
int prevTime = 0;

String serverName = "https://timezone.abstractapi.com/v1/current_time?api_key=9aeecd1a024f4daba73b4081054a8168&location=Denver, Colorado"; // Just a free API I found to use that lets me make 5000 calls for the time for free a month

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Ripped code from one of my old projects, i think it still works!
// TODO: It currently uses a bad string parser which should use a json parser
void makeCall(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String serverPath = serverName;

    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    int tHour = 0;
    int tMinute = 0;
    int tSecond = 0;

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      String time12 = payload.substring(24, 32);
      Serial.println(time12);
      tHour = (time12.substring(0, 2)).toInt();
      tMinute = (time12.substring(3, 5)).toInt();
      tSecond = (time12.substring(6)).toInt();
      if (tHour >= 13) {
        tHour = tHour - 12;
      }
      if (tHour == 0) {
        tHour = 12;
      }
      Serial.println(tHour);
      Serial.println(tMinute);
      Serial.println(tSecond);
      Serial.println(payload);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
    timeHour = tHour;
    timeMinute = tMinute;
    timeSecond = tSecond;
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Starting");
  delay(5000);
  lcd.clear();
  lcd.noDisplay();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  makeCall();
  randomSeed(timeHour);
  timeUntilReset = ((59-timeMinute)*60000) + ((59-timeSecond)*1000);
}

void loop() {
  bool button_state = digitalRead(BUTTON_PIN);
  if(buttonPresses >= OVERFLOW){
    randomSeed(timeHour); // Reset the seed if overflow
    buttonPresses = 0;
  }
  if(button_state == false){  // False because button is pulled up
    lcd.display(); // Turn LCD backlight on
    delay(500);
    lcd.print(random(100000, 999999));
    delay(10000);
    lcd.clear();
    lcd.noDisplay(); // Turn LCD off to prevent longterm damage
    delay(2000);
    buttonPresses++;
  }
  if(millis() > timeUntilReset || (millis() + timeUntilReset) > (prevTime + 3600000)){
    makeCall();
    randomSeed(timeHour);
    prevTime = millis();
    buttonPresses = 0;
  }
  delay(10);
}

