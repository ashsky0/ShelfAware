#include "HX711.h"

// Pin definitions
#define DOUT  18  // Connect to HX711 DOUT
#define SCK   19  // Connect to HX711 SCK

HX711 scale;

float calibration_factor = 0; //-7050.0; // Calibrate this!

void setup() {
  Serial0.begin(9600);
  Serial0.println("HX711 Calibration Test");

  scale.begin(DOUT, SCK);

  Serial0.println("Remove all weight from the scale");
  delay(3000);
  scale.tare();  // Reset the scale to 0
  Serial0.println("Scale tared. Place a known weight on the scale.");
}


  long reading = 0;
  long prev_reading = 10000;

void loop() {
  if (scale.is_ready()) {
    reading = scale.get_units(20); // Average of 10 readings
    Serial0.print("Reading: ");
    Serial0.println(reading);

    if(reading > (prev_reading + (reading / 2))) {
      Serial0.println("New Item?");
      Serial0.println(reading);
      Serial0.println(prev_reading);
    }
    else if (reading < (prev_reading - (reading / 2))) {
      Serial0.println("Item Removed?");
      Serial0.println(reading);
      Serial0.println(prev_reading);
    }

    prev_reading = reading;

    delay(1000);
  } else {
    Serial0.println("HX711 not found.");
  }
}
