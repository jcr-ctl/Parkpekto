// Define pins for ultrasonic sensors
const int trigPins[6] = {9, 8, 7, 6, 5, 4}; // Trig pins for 6 sensors
const int echoPins[6] = {A0, A1, A2, A3, A4, A5}; // Echo pins for 6 sensors

// Define pins for LEDs
const int greenLEDPins[3] = {10, 11, 12}; // Green LED pins for 3 pairs of sensors
const int redLEDPins[3] = {13, 2, 3};     // Red LED pins for 3 pairs of sensors

// Define acceptable distance ranges (in cm)
const int minDistance = 10;
const int maxDistance = 20;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Set up pins for ultrasonic sensors
  for (int i = 0; i < 6; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }
  
  // Set up pins for LEDs
  for (int i = 0; i < 3; i++) {
    pinMode(greenLEDPins[i], OUTPUT);
    pinMode(redLEDPins[i], OUTPUT);
    
    // Initialize LEDs to be off
    digitalWrite(greenLEDPins[i], LOW);
    digitalWrite(redLEDPins[i], LOW);
  }
}

void loop() {
  // Measure distances for each sensor
  int distances[6];
  for (int i = 0; i < 6; i++) {
    distances[i] = measureDistance(trigPins[i], echoPins[i]);
  }
  
  // Check each pair of sensors and determine status
  String status[3];
  for (int i = 0; i < 3; i++) {
    int sensor1 = 2 * i;
    int sensor2 = 2 * i + 1;
    status[i] = getParkingStatus(distances[sensor1], distances[sensor2]);
    updateLEDs(i, status[i]);
  }
  
  // Send JSON data to serial
  String jsonData = "{";
  for (int i = 0; i < 3; i++) {
    jsonData += "\"slot" + String(i+1) + "\":\"" + status[i] + "\"";
    if (i < 2) {
      jsonData += ",";
    }
  }
  jsonData += "}";
  
  Serial.println(jsonData);
  
  // Wait for a short period before next measurement
  delay(1000);
}

// Function to measure distance using ultrasonic sensor
int measureDistance(int trigPin, int echoPin) {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Set the trigPin on HIGH state for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin, returns the sound wave travel time in microseconds
  unsigned long duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance in centimeters
  int distance = duration * 0.034 / 2;
  
  return distance;
}

// Function to get parking status based on sensor distances
String getParkingStatus(int distance1, int distance2) {
  if (distance1 > minDistance && distance2 > minDistance) {
    return "vacant";
  } else if (distance1 <= minDistance && distance2 <= minDistance && abs(distance1 - distance2) <= 3) {
    return "occupied";
  } else if ((distance1 <= minDistance && distance2 > minDistance) || (distance2 <= minDistance && distance1 > minDistance)) {
    return "misaligned";
  } else {
    return "error";
  }
}

// Function to update LEDs based on parking slot status
void updateLEDs(int pairIndex, String status) {
  int greenPin = greenLEDPins[pairIndex];
  int redPin = redLEDPins[pairIndex];
  
  if (status == "vacant") {
    // Vacant space
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);
  } else if (status == "occupied") {
    // Properly parked
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
  } else if (status == "misaligned") {
    // Wrong parking
    digitalWrite(greenPin, LOW);
    blinkRedLED(redPin);
  } else {
    // No vehicle detected or error state
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
  }
}

// Function to blink the red LED indicating a misalignment/error
void blinkRedLED(int pin) {
  digitalWrite(pin, HIGH);
  delay(250);
  digitalWrite(pin, LOW);
  delay(250);
}
