//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

// LIBRARY IMPORTS
#include <rom/rtc.h>

#ifndef _WIFI_H 
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

// DEFINE VARIABLES
#define ARDUINOJSON_USE_DOUBLE      1 
// DEFINE THE PINS THAT WILL BE MAPPED TO THE 7 SEG DISPLAY BELOW, 'a' to 'g'
#define a     15
#define b     32
#define c     33
#define d     25
#define e     26
#define f     27
#define g     14
#define dp    12
#define LED_A 4
#define LED_B 19
#define BTN_A 5


// MQTT CLIENT CONFIG  
static const char* pubtopic       = "620160532";                    // Add your ID number here
static const char* subtopic[]     = {"620160532_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server    = "broker.emqx.io";                // Broker IP address or Domain name as a String 
static uint16_t mqtt_port         = 1883;

// WIFI CREDENTIALS
const char* ssid                  = "Mike"; // Add your Wi-Fi ssid
const char* password              = "micheal12"; // Add your Wi-Fi password 




// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL; 

// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters ); 
void GDP(void);   // GENERATE DISPLAY PUBLISH

/* Declare your functions below */
void Display(unsigned char number);
int8_t getLEDStatus(int8_t LED);
void setLEDState(int8_t LED, int8_t state);
void toggleLED(int8_t LED);
  

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif

// Temporary Variables
uint8_t number = 0;


void setup() {
  Serial.begin(115200);  // INIT SERIAL 
  randomSeed(analogRead(0)); 

  // Connect to Wi-Fi
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);

    // Wait for connection
    


  // CONFIGURE THE ARDUINO PINS OF THE 7SEG AS OUTPUT
  pinMode(a,OUTPUT);
  pinMode(b,OUTPUT);
  pinMode(c,OUTPUT);
  pinMode(d,OUTPUT);
  pinMode(e,OUTPUT);
  pinMode(f,OUTPUT);
  pinMode(g,OUTPUT);
  pinMode(dp,OUTPUT);
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BTN_A, INPUT_PULLUP);

  Display(8);
  vButtonCheckFunction(); // UNCOMMENT IF USING BUTTONS THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION
  //Display(8);


  initialize();           // INIT WIFI, MQTT & NTP 
  
}
  


void loop() {
    
    
}




  
//####################################################################
//#                          UTIL FUNCTIONS                          #       
//####################################################################
void vButtonCheck( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );     
      
    for( ;; ) {

      // Check if the button is pressed (LOW for a pull-up configuration)
        if (digitalRead(BTN_A) == LOW) {
          delay(500);
          GDP();
            // Generate a random number between 0 and 9
            //int randomNumber = random(0, 10);

            // Display the random number on a 7-segment display
            //Display(randomNumber);

            // Small delay to prevent multiple detections from button bounce
            //vTaskDelay(200 / portTICK_PERIOD_MS);
        }
        // then execute appropriate function if a button is pressed  

       // vTaskDelay(200 / portTICK_PERIOD_MS);  
    }
            vTaskDelay(200 / portTICK_PERIOD_MS);  

}

void vUpdate( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );    
           
    for( ;; ) {
          // Task code goes here.   
          // PUBLISH to topic every second.
          JsonDocument doc; // Create JSon object
          char message[1100]  = {0};

          // Add key:value pairs to JSon object
          doc["id"]         = "620012345";

          serializeJson(doc, message);  // Seralize / Covert JSon object to JSon string and store in char* array

          if(mqtt.connected() ){
            publish(pubtopic, message);
          }
          
            
        vTaskDelay(1000 / portTICK_PERIOD_MS);  
    }
}

unsigned long getTimeStamp(void) {
          // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
          time_t now;         
          time(&now); // Retrieve time[Timestamp] from system and save to &now variable
          return now;
}

void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  
  for (int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, received);  

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }


  // PROCESS MESSAGE
  const char* type = doc["type"];
  //Serial.print("Type: ");
  //Serial.println(type);

  if (strcmp(type, "toggle") == 0){
    // Process messages with ‘{"type": "toggle", "device": "LED A"}’ Schema
    const char* led = doc["device"];

    if(strcmp(led, "LED A") == 0){
      /*Add code to toggle LED A with appropriate function*/
      toggleLED(LED_A);
    }
    if(strcmp(led, "LED B") == 0){
      /*Add code to toggle LED B with appropriate function*/
      toggleLED(LED_B);
    }

    // PUBLISH UPDATE BACK TO FRONTEND
    JsonDocument doc; // Create JSon object
    char message[800]  = {0};

    // Add key:value pairs to Json object according to below schema
    // ‘{"id": "student_id", "timestamp": 1702212234, "number": 9, "ledA": 0, "ledB": 0}’
    doc["id"]         = "620160532"; // Change to your student ID number
    doc["timestamp"]  = getTimeStamp();
    doc["number"] = number;
    doc["ledA"] = getLEDStatus(LED_A);
    doc["ledB"] = getLEDStatus(LED_B);
    /*Add code here to insert all other variabes that are missing from Json object
    according to schema above
    */

    serializeJson(doc, message);  // Seralize / Covert JSon object to JSon string and store in char* array  
    publish("620160532", message);    // Publish to a topic that only the Frontend subscribes to.

  } 

}

bool publish(const char *topic, const char *payload){   
     bool res = false;
     try{
        res = mqtt.publish(topic,payload);
        //Serial.printf("yes\n");
        // Serial.printf("\nres : %d\n",res);
        if(!res){
          res = false;
          throw false;
        }
     }
     catch(...){
      Serial.printf("\nError (%d) >> Unable to publish message\n", res);
     }
  return res;
}

//***** Complete the util functions below ******

void Display(unsigned char number){

// Set all segments to LOW (turn off the display)
    digitalWrite(a, LOW);
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(d, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    digitalWrite(g, LOW);
    digitalWrite(dp, LOW); // Decimal point off

    // Check the number and set the segments accordingly
    if (number == 0) {
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(e, HIGH);
        digitalWrite(f, HIGH);
    } else if (number == 1) {
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
    } else if (number == 2) {
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(e, HIGH);
        digitalWrite(g, HIGH);
    } else if (number == 3) {
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(g, HIGH);
    } else if (number == 4) {
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
    } else if (number == 5) {
        digitalWrite(a, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
    } else if (number == 6) {
        digitalWrite(a, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(e, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
    } else if (number == 7) {
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
    } else if (number == 8) {
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(e, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
    } else if (number == 9) {
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
        digitalWrite(c, HIGH);
        digitalWrite(d, HIGH);
        digitalWrite(f, HIGH);
        digitalWrite(g, HIGH);
    }  
}

int8_t getLEDStatus(int8_t LED) {
  return digitalRead(LED); 
}

void setLEDState(int8_t LED, int8_t state){
  if (state == HIGH) {
        digitalWrite(LED, HIGH);  // Turn the LED ON
    } else if (state == LOW) {
        digitalWrite(LED, LOW);   // Turn the LED OFF
    }
}

void toggleLED(int8_t LED){
  // Read the current state of the LED
    int8_t currentState = digitalRead(LED);

    // Toggle the LED state
    if (currentState == HIGH) {
        digitalWrite(LED, LOW);  // If it's HIGH, set it to LOW (turn off)
    } else {
        digitalWrite(LED, HIGH); // If it's LOW, set it to HIGH (turn on)
    }
}

void GDP(void){
  
  int number = random(0, 10); 
  Display(number);

  // PUBLISH number to topic.
  JsonDocument doc; // Create JSon object
  char message[1100]  = {0};

  // Add key:value pairs to Json object according to below schema
  // ‘{"id": "student_id", "timestamp": 1702212234, "number": 9, "ledA": 0, "ledB": 0}’
  doc["id"]         = "620160532"; // Change to your student ID number
  doc["timestamp"]  = getTimeStamp();
  doc["number"]     = number;
  doc["ledA"]       = getLEDStatus(LED_A); // Read the state of LED_A (HIGH or LOW)
  doc["ledB"]       = getLEDStatus(LED_B); // Read the state of LED_B (HIGH or LOW)

  /*Add code here to insert all other variabes that are missing from Json object
  according to schema above
  */

  serializeJson(doc, message);  // Seralize / Covert JSon object to JSon string and store in char* array
  publish(pubtopic, message);

}