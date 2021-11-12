const int trigPin = 4;
const int echoPin = 18;
const int buzzerPin = 32;

// define sound speed in cm per microsecond
#define SOUND_SPEED 0.034

long duration;
float distanceCm;

float surfaceArea;    // Surface area of the bucket in square cm
float bucketHeight;   // Height of the bucket in cm
float triggerHeight;  // 90% of bucketHeight at which the buzzer is triggered to warn the user

int lastHeight;       // Stores the previous water level in cm for calculating the flow of the water
int flow;             // Stores the increase in water level per loop

int overflowHeight;   // Stores the height the sensor will read when the bucket is full 
int overflowTime;     // Stores the number of milliseconds for which the bucket overflowed
int overflow;         // 1 if bucket has overflown, else 0

void setup() {
  Serial.begin(9600); // Starts the serial communication
  pinMode(trigPin, OUTPUT);   // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);    // Sets the echoPin as an Input
  pinMode(buzzerPin, OUTPUT); // Sets the buzzerPin as Output

  digitalWrite(buzzerPin,LOW);  // Sets buzzerPin as LOW initially
  overflowTime = 0;

  // Take Surface Area as input from the user
  Serial.println("Enter approximate surface area of the bucket(in square cm): ");
  if (Serial.available() > 0) {
    surfaceArea = Serial.read();
    Serial.print("Surface Area is: ");
    Serial.println(surfaceArea, DEC);
  }

  // Callibrate bucket height
  Serial.println("Hold the sensor at the rim of the bucket to callibrate the height of the bucket");
  Serial.println("Enter any key to callibrate");
  Serial.println("You will receive the warning when water level reaches 90% of the bucket height");
  if (Serial.available() > 0) {
    Serial.read();
    
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    
    // Calculate the height
    bucketHeight = duration * SOUND_SPEED/2;
    triggerHeigth = bucketHeigth/10;
  }
  
}

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
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  if(distanceCm <= triggerHeigth) {  
    digitalWrite(buzzerPin, HIGH);
    Serial.println("WARNING: Bucket about to overflow");
    flow = lastHeight - distanceCm;
  }
  else {
    digitalWrite(buzzerPin, LOW);
  }

  if(distanceCm <= overflowHeight) {
    overflowTime += 1;
    overflow = 1;
  }

  if(overflow == 1) {
    float volumeWasted = overflowTime * flow;
    Serial.print("Volume of water wasted (in square cm) is: ");
    Serial.println(volumeWasted, DEC);
  }

  // This if condition will be executed after the user stops the buzzer or flow of water
  // Currently, this is triggered if the user enters 1 in the Serial Monitor
  if (Serial.available() > 0) {
    int input = (int)(Serial.read() - '0');
    overflow = 0;
    // exit code
  }
  

  lastHeight = distanceCm;
  delay(50);
}
