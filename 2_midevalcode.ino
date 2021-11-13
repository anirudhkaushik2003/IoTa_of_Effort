#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "ThingSpeak.h"

char* writeAPIKey = "2ANIGQ9PDMBMIU00";
char* readAPIKey = "IUKMEMLTTQDJMNU1"; 
const long channelID = 1569938;


// Replace with your network credentials
const char* ssid = "RDJ";
const char* password = "12345678";

WiFiClient client; 




// Set LED GPIO
const int ledPin = 2;
// Stores LED state
String ledState;

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input-height";
const char* PARAM_INPUT_3 = "input3";

int GLOBAL_FLAG = 1;
float bucketHeight; 
float triggerHeight;
float surfaceArea;
float distanceCm;
const int trigPin = 4;
const int echoPin = 12;
const int buzzerPin = 32;

// define sound speed in cm per microsecond
#define SOUND_SPEED 0.034

long duration;
float lastHeight;       // Stores the previous water level in cm for calculating the flow of the water
float flow;             // Stores the increase in water level per loop

float overflowHeight;   // Stores the height the sensor will read when the bucket is full 
int overflowTime;     // Stores the number of milliseconds for which the bucket overflowed
float overflow;         // 1 if bucket has overflown, else 0

float volumeWasted;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  return String();
}


 
void setup(){
  // Serial port for debugging purposes
  

  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT);   // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);    // Sets the echoPin as an Input
  pinMode(buzzerPin, OUTPUT); // Sets the buzzerPin as Output

  digitalWrite(buzzerPin,LOW);  // Sets buzzerPin as LOW initially
  overflowTime = 0;


  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());


  ThingSpeak.begin(client);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  

  // Route to set GPIO to HIGH
  server.on("/calibrate_mode", HTTP_GET, [](AsyncWebServerRequest *request){ 
    bucketHeight = distanceCm;
    Serial.println(bucketHeight);   
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
file:///set_height?input1=56
  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/set_height", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      triggerHeight = inputMessage.toFloat();
      Serial.println(triggerHeight);
      inputParam = PARAM_INPUT_1;
    }

    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  
  
  // Route to set GPIO to LOW
  server.on("/set-overflow-height.html", HTTP_GET, [](AsyncWebServerRequest *request){
       
    request->send(SPIFFS, "/set-overflow-height.html", String(), false, processor);
  });
  server.on("/set-height.html", HTTP_GET, [](AsyncWebServerRequest *request){
       
    request->send(SPIFFS, "/set-height.html", String(), false, processor);
  });

  server.on("/get2", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      overflowHeight = inputMessage.toFloat();
      Serial.println(overflowHeight);
      inputParam = PARAM_INPUT_2;
    }

    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });

  server.on("/surface-area.html", HTTP_GET, [](AsyncWebServerRequest *request){
       
    request->send(SPIFFS, "/surface-area.html", String(), false, processor);
  });
  
  server.on("/get3", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      surfaceArea = inputMessage.toFloat();
      Serial.println(surfaceArea);
      inputParam = PARAM_INPUT_1;
    }

    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });

  // Start server
  server.begin();
}
 
//void loop() {
//  // Clears the trigPin
//  digitalWrite(trigPin, LOW);
//  delayMicroseconds(2);
//  // Sets the trigPin on HIGH state for 10 micro seconds
//  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(10);
//  digitalWrite(trigPin, LOW);
//  
//  // Reads the echoPin, returns the sound wave travel time in microseconds
//  duration = pulseIn(echoPin, HIGH);
//  
//  // Calculate the distance
//  distanceCm = duration * SOUND_SPEED/2;
//  
//  // Prints the distance in the Serial Monitor
//  Serial.print("Distance (cm): ");
//  Serial.println(distanceCm);
//
//  if(distanceCm <= triggerHeight) {  
//    digitalWrite(buzzerPin, HIGH);
//    Serial.println("WARNING: Bucket about to overflow");
//    flow = lastHeight - distanceCm;
//  }
//  else {
//    digitalWrite(buzzerPin, LOW);
//  }
//
//  if(distanceCm <= overflowHeight) {
//    overflowTime += 1;
//    overflow = 1;
//  }
//
//  if(overflow == 1) {
//    volumeWasted = overflowTime * flow;
//    Serial.print("Volume of water wasted (in cubic cm) is: ");
//    Serial.println(volumeWasted, DEC);
//  }
//
//  // This if condition will be executed after the user stops the buzzer or flow of water
//  // Currently, this is triggered if the user enters 1 in the Serial Monitor
//  if (Serial.available() > 0) {
//    int input = (int)(Serial.read() - '0');
//    overflow = 0;
//    digitalWrite(buzzerPin, LOW);
//    // exit code
//  }
//  
//
//  lastHeight = distanceCm;
//  delay(500);
//}


void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;

  if(distanceCm <= bucketHeight && GLOBAL_FLAG == 1)
  {
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);




    if(distanceCm <= overflowHeight) {
    overflowTime += 1;
    overflow = 1;
  }

  else 
  { 
    overflow = 0;
    overflowTime = 0;

  }


  if (overflow != 1) 
    {
      flow = lastHeight - distanceCm;
      Serial.println(flow);
    }


   
  if(distanceCm <= triggerHeight) {  
    digitalWrite(buzzerPin, HIGH);
    Serial.println("WARNING: Bucket about to overflow");

    
  }
  else {
    digitalWrite(buzzerPin, LOW);
  }
  

  if(overflow == 1) {
    volumeWasted = overflowTime * flow * surfaceArea;
    Serial.print("Volume of water wasted (in cubic cm) is: ");
    Serial.println(volumeWasted, DEC);
  }

  // This if condition will be executed after the user stops the buzzer or flow of water
  // Currently, this is triggered if the user enters 1 in the Serial Monitor
  if (Serial.available() > 0) {
    int input = (int)(Serial.read() - '0');
    overflow = 0;
    digitalWrite(buzzerPin, LOW);
    // GLOBAL_FLAG = 0;
    // exit(0);
  }
  

  lastHeight = distanceCm;
  delay(500);
  ThingSpeak.writeField(channelID,1,distanceCm,writeAPIKey);
  ThingSpeak.writeField(channelID,2,volumeWasted,writeAPIKey);
  }


  
  
  

}
  
