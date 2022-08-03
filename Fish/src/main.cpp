#include <Arduino.h>

#include <esp_now.h>
#include <WiFi.h>
#include <Servo.h>

Servo servo;
int pos = 0;

int prevTime = 0;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
int time;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  // Write code here what needs to happen when fish toggled. Can also implement lights and other motion
  
  prevTime = millis();
  while(millis() < prevTime + myData.time){
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      servo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      servo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
  }
}
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  servo.attach(9); // What pin to attach the servo to
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

}