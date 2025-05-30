#include "HX711.h"

// Pin definitions
#define DOUT  18  // Connect to HX711 DOUT
#define SCK   19  // Connect to HX711 SCK

#define SCALE_COEFF 338.310394
#define SCALE_OFFSET 1412837

HX711 scale;

float calibration_factor = 0; //-7050.0; // Calibrate this!

void setup() {
  Serial0.begin(115200);
  Serial0.println("HX711 Calibration Test");

  scale.begin(DOUT, SCK);

  Serial0.println("Remove all weight from the scale");
  delay(3000);
  //scale.tare();  // Reset the scale to 0
  scale.set_offset(SCALE_OFFSET); 
  scale.set_scale(SCALE_COEFF);
}


  long reading = 0;
  long prev_reading = 10000;

void loop() {
  if (scale.is_ready()) {
    reading = scale.get_units(); // Average of 10 readings
    //Serial0.print("Reading: ");
    Serial0.println(reading);

    if(reading > (prev_reading + (reading / 2))) {
      //Serial0.println("New Item?");
    }
    else if (reading < (prev_reading - (reading / 2))) {
      //Serial0.println("Item Removed?");
    }

    prev_reading = reading;

    delay(1);
  } else {
    //Serial0.println("HX711 not found.");
  }
}
