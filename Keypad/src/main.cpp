#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h> // It is completely possible to do this without this library, but I think it makes things simpler. If you want later down the line you could manually register each coordinate position and map it to each key. 
#include <esp_now.h>

// To validate a code, the program generates a procedural random sequence of numbers based on the date which it matched to the keypad
// To ensure it does not exceed its memory limit, the numbers in the procedural generation sequence are used as checks multiple times
// Valid codes reset every hour, but give 5334118 a try...
// This sketch uses some weird 2D array logic and WIFI settings (connection to internet and STA mode at the same time) so it may not work properly without tweaks. I dont have anything to test it but can get some equipment later

// Global varibles galore. I paid for the global memory so im going to use global varibles

#define OVERFLOW 840 // How many codes to present before repeating in an hour. With 840 there is a 0.09% chance they guess a correct code with 899999 possibillities. Must be the same as on the LCD

#define FISH_TIME 20000 // How long in ms for the fish to do fish shit

const int KEYPAD_ROW_NUM = 4; //four rows
const int KEYPAD_COLUMN_NUM = 3; //three columns

char keys[KEYPAD_ROW_NUM][KEYPAD_COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pin_rows[KEYPAD_ROW_NUM] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte pin_column[KEYPAD_COLUMN_NUM] = {5, 4, 3}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, KEYPAD_ROW_NUM, KEYPAD_COLUMN_NUM );

String userPassword = "";

const int BROADCAST_ROW_NUM = 4; // Number of ESP fish
const int BROADCAST_COLOUMN_NUM = 6; // Useless i guess but number of mac address indexes per address
uint8_t addresses[BROADCAST_ROW_NUM][BROADCAST_COLOUMN_NUM] = {
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

typedef struct struct_message {
  int time;
} struct_message;

// Wifi can not have a captive gateway
const char* ssid = ""; // Replace with your WIFI SSID
const char* password = ""; // Your WIFI Password

int timeHour = 0;
int timeMinute = 0;
int timeSecond = 0;

int timeUntilReset = 0;
int prevTime = 0;

String serverName = "https://timezone.abstractapi.com/v1/current_time?api_key=9aeecd1a024f4daba73b4081054a8168&location=Denver, Colorado"; // Just a free API I found to use that lets me make 5000 calls for the time for free a month

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

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

bool isValidCode(){
  randomSeed(timeHour);
  for(int i = 0; i < OVERFLOW; i++){
    if(String(random(100000, 999999)) == userPassword){ // Smooth casting criminal
      return true;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  for(int i = 0; i < BROADCAST_ROW_NUM; i++){
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, addresses[i], 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

  makeCall();
  randomSeed(timeHour);
  timeUntilReset = ((59-timeMinute)*60000) + ((59-timeSecond)*1000);
}

void loop() {
  char key = keypad.getKey();
  if(key == '*'){
    userPassword = "";
  }else if(key == '#'){
    if(isValidCode() == true){
      // toggle fish
      // Create a struct_message called myData
      struct_message myData;
      myData.time = FISH_TIME;
      for(int i = 0; i < BROADCAST_ROW_NUM; i++){
        esp_err_t result = esp_now_send(addresses[i], (uint8_t *) &myData, sizeof(myData));
        if (result == ESP_OK) {
          Serial.println("Sent with success");
        }else {
          Serial.println("Error sending the data");
        }
        delay(FISH_TIME + 1000);
      }
      userPassword = "";
    }else if(userPassword == "5334118"){
      // do somthing super cool/test constant code

    }else if(userPassword == "6445229"){
      // is there a third code in the pattern? people will try!

    }else{
      userPassword = "";
    }
  }else{
    userPassword += key;
  }

  if(millis() > timeUntilReset || (millis() + timeUntilReset) > (prevTime + 3600000)){
    makeCall();
    randomSeed(timeHour);
    prevTime = millis();
  }
  delay(10);
}