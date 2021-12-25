#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "ThingSpeak.h"
#include "HTTPClient.h"
#include "time.h"
#include <ArduinoJson.h>

int playing = 0;
int r = 1000;
void tone(byte pin, int freq)
{
    ledcSetup(0, 2000, 8);  // setup beeper
    ledcAttachPin(pin, 0);  // attach beeper
    ledcWriteTone(0, freq); // play tone
    playing = pin;          // store pin
}
void noTone()
{
    tone(playing, 0);
}
// notes in the melody:


// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4};

String cse_ip = "192.168.0.104"; // YOUR IP from ipconfig/ifconfig
String cse_port = "8080";
String server_om2m = "http://" + cse_ip + ":" + cse_port + "/~/in-cse/in-name/";
String ae = "HC_SR04_IoTa_of_Effort";
String cnt = "distancenode";
String cnt2 = "volumeNode";

char *writeAPIKey = "2ANIGQ9PDMBMIU00";
char *readAPIKey = "IUKMEMLTTQDJMNU1";
const long channelID = 1569938;

// Replace with your network credentials
const char *ssid = "RDJ";
const char *password = "12345678";

WiFiClient client;

// Enter the Auth Token provied by Blynk app
#define BLYNK_TEMPLATE_ID "TMPLGKTseSdc"
#define BLYNK_DEVICE_NAME "Quickstart Template"
char auth[] = "N3Awoi5BEgRFFRl1PM_CeHnNjrw9CGHp";
#include <BlynkSimpleEsp32.h>

void createCI(String &val)
{
    HTTPClient http;
    http.begin(server_om2m + ae + "/" + cnt + "/");
    http.addHeader("X-M2M-Origin", "admin:admin");
    http.addHeader("Content-Type", "application/json;ty=4");
    int code = http.POST("{\"m2m:cin\": {\"cnf\":\"application/json\",\"con\": " + String(val) + "}}");
    Serial.println(code);
    if (code == -1)
    {
        delay(1000);
        Serial.println("UNABLE TO CONNECT TO THE SERVER");
    }
    http.end();
}

void createCI2(String &val)
{
    HTTPClient http;
    http.begin(server_om2m + ae + "/" + cnt2 + "/");
    http.addHeader("X-M2M-Origin", "admin:admin");
    http.addHeader("Content-Type", "application/json;ty=4");
    int code = http.POST("{\"m2m:cin\": {\"cnf\":\"application/json\",\"con\": " + String(val) + "}}");
    Serial.println(code);
    if (code == -1)
    {
        delay(1000);
        Serial.println("UNABLE TO CONNECT TO THE SERVER");
    }
    http.end();
}

// Set LED GPIO
const int ledPin = 2;
// Stores LED state
String ledState;

const char *PARAM_INPUT_1 = "input1";
const char *PARAM_INPUT_2 = "input-height";
const char *PARAM_INPUT_3 = "input3";

int GLOBAL_FLAG = 1;
float bucketHeight = -1;
float triggerHeight;
float surfaceArea;
float distanceCm;
const int trigPin = 4;
const int echoPin = 14;
const int buzzerPin = 32;

// define sound speed in cm per microsecond
#define SOUND_SPEED 0.034

long duration;
float lastHeight; // Stores the previous water level in cm for calculating the flow of the water
float flow;       // Stores the increase in water level per loop

float overflowHeight = 5; // Stores the height the sensor will read when the bucket is full
int overflowTime;         // Stores the number of milliseconds for which the bucket overflowed
int overflow;             // 1 if bucket has overflown, else 0

float volumeWasted;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String &var)
{
    // Serial.println(var);
    if (var == "STATE")
    {
        if (digitalRead(ledPin))
        {
            ledState = "ON";
        }
        else
        {
            ledState = "OFF";
        }
        // Serial.print(ledState);
        return ledState;
    }
    return String();
}

void setup()
{
    // Serial port for debugging purposes

    Serial.begin(115200); // Starts the serial communication
    

    pinMode(trigPin, OUTPUT);   // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);    // Sets the echoPin as an Input
    pinMode(buzzerPin, OUTPUT); // Sets the buzzerPin as Output

    digitalWrite(buzzerPin, LOW); // Sets buzzerPin as LOW initially
    overflowTime = 0;

    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    Blynk.begin(auth, ssid, password);

    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());

    ThingSpeak.begin(client);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", String(), false, processor); });

    // Route to load style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/style.css", "text/css"); });

    server.on("/style2.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/style2.css", "text/css"); });

    server.on("/img/Droplet-Circle-Loader.gif", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/img/Droplet-Circle-Loader.gif", "image/gif"); });

    //    server.on("/img/unknown.png", HTTP_GET, [](AsyncWebServerRequest *request)
    //              { request->send(SPIFFS, "/img/unknown.png", "image/png"); });

    server.on("/img/calibrate.png", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/img/calibrate.png", "image/png"); });

    server.on("/img/bucket.png", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/img/bucket.png", "image/png"); });

    server.on("/img/water.png", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/img/water.png", "image/png"); });

    server.on("/img/bucket2.png", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/img/bucket2.png", "image/png"); });

    server.on("/img/bg.jpg", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/img/bg.jpg", "image/jpg"); });

    // Route to set GPIO to HIGH
    server.on("/calibrate_mode", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  bucketHeight = distanceCm - overflowHeight;

                  Serial.println(bucketHeight);
                  request->send(SPIFFS, "/index.html", String(), false, processor);
              });
    // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
    server.on("/set_height", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  String inputMessage;
                  String inputParam;
                  // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
                  if (request->hasParam(PARAM_INPUT_1))
                  {
                      inputMessage = request->getParam(PARAM_INPUT_1)->value();
                      float percentage;
                      percentage = inputMessage.toFloat();
                      if (percentage <= 95 && percentage >= 0)
                      {
                          triggerHeight = (bucketHeight * (100 - percentage)) / 100.0;
                      }
                      else
                      {
                          inputMessage = "Not a valid percentage";
                      }
                      Serial.println(triggerHeight);
                      inputParam = PARAM_INPUT_1;
                  }

                  else
                  {
                      inputMessage = "No message sent";
                      inputParam = "none";
                  }
                  Serial.println(inputMessage);
                  request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" + inputParam + ") with value: " + inputMessage + "<br><a href=\"/\">Return to Home Page</a>");
              });

    server.on("/set-height.html", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/set-height.html", String(), false, processor); });

    server.on("/surface-area.html", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  surfaceArea = 3.14 * pow((distanceCm / 2), 2);
                  Serial.print("Surface Area(cm^2): ");
                  Serial.println(surfaceArea);
                  request->send(SPIFFS, "/index.html", String(), false, processor);
              });

    // Start server
    server.begin();
    Blynk.notify("Water overflowing");
}

void loop()
{

    server.begin();

    //   if(bucketHeight > 0) {
    Blynk.run();
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
    distanceCm = duration * SOUND_SPEED / 2;

    if (distanceCm <= bucketHeight && GLOBAL_FLAG == 1)
    {

        // Prints the distance in the Serial Monitor
        Serial.print("Sensor reading (cm) ");
        Serial.println(distanceCm);

        Serial.print("height left (cm) ");
        Serial.println(distanceCm - overflowHeight);

        if (distanceCm <= overflowHeight)
        {
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
            if (lastHeight - distanceCm > 0)
            {
                flow = lastHeight - distanceCm;
            }
            // Serial.println(flow);
        }

        if ((distanceCm - overflowHeight) <= triggerHeight && bucketHeight > 0)
        {
            tone(buzzerPin, r);
            Serial.println("WARNING: Bucket about to overflow");
            Serial.println(triggerHeight);
            server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                      { request->send(SPIFFS, "/index2.html", String(), false, processor); });
        }


        if (overflow == 1)
        {
            Blynk.notify("Water overflowing");

            volumeWasted = overflowTime * flow * surfaceArea;
            Serial.print("Volume of water wasted (in cubic cm) is: ");

            Serial.print(volumeWasted, DEC);
        }

        // This if condition will be executed after the user stops the buzzer or flow of water
        // Currently, this is triggered if the user enters 1 in the Serial Monitor
        if (Serial.available() > 0)
        {
            int input = (int)(Serial.read() - '0');
            overflow = 0;
            digitalWrite(buzzerPin, LOW);
            // GLOBAL_FLAG = 0;
            // exit(0);
        }

        lastHeight = distanceCm;
        delay(500);
        noTone();
        ThingSpeak.writeField(channelID, 1, distanceCm, writeAPIKey);
        ThingSpeak.writeField(channelID, 2, volumeWasted, writeAPIKey);

        String string1 = String(distanceCm);
        createCI(string1);
        String string2 = String(volumeWasted);
        createCI2(string2);
    }
    // }
}