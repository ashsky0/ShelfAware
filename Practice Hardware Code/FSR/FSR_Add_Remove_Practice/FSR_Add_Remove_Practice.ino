#define FSR_PIN 1          // Use a valid ADC1 pin on ESP32-S3
#define THRESHOLD 50       // Sensitivity to detect object change

int baseline = 0;
bool objectPresent = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  baseline = analogRead(FSR_PIN);
  Serial.print("Baseline: ");
  Serial.println(baseline);
}

void loop() {
  int reading = analogRead(FSR_PIN);
  int diff = reading - baseline;
  unsigned long currentTime = millis();  // Time since startup in ms

  if (!objectPresent && diff > THRESHOLD) {
    objectPresent = true;
    Serial.print("Object added at ");
    Serial.print(currentTime / 1000.0, 2); // Convert to seconds
    Serial.println(" seconds");
  } 
  else if (objectPresent && diff < THRESHOLD / 2) {
    objectPresent = false;
    Serial.print("Object removed at ");
    Serial.print(currentTime / 1000.0, 2);
    Serial.println(" seconds");
  }

  delay(200); // debounce / reduce spam
}
