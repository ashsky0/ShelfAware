#define GPIO_PIN 1

void setup() {
  // put your setup code here, to run once:
  delay(2000);
  Serial0.begin(115200);

  //pinMode(GPIO_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int fsrReading = analogRead(GPIO_PIN);

  // Print the value to the serial monitor
  Serial0.print("Analog reading = ");
  Serial0.println(fsrReading);

  // Add a small delay to avoid overwhelming the serial monitor
  delay(100);
}
