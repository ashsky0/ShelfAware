//
//    FILE: HX_calibration.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: HX711 calibration finder for offset and scale
//     URL: https://github.com/RobTillaart/HX711


#include "HX711.h"

HX711 myScale;

//  adjust pins if needed.
uint8_t dataPin = 18;
uint8_t clockPin = 19;


void setup()
{
  Serial0.begin(115200);
  delay(3000);
  Serial0.println(__FILE__);
  Serial0.print("HX711_LIB_VERSION: ");
  Serial0.println(HX711_LIB_VERSION);
  Serial0.println();

  myScale.begin(dataPin, clockPin);
}

void loop()
{
  calibrate();
}



void calibrate()
{
  Serial0.println("\n\nCALIBRATION\n===========");
  Serial0.println("remove all weight from the loadcell");
  //  flush Serial input
  while (Serial0.available()) Serial0.read();

  Serial0.println("and press enter\n");
  while (Serial0.available() == 0);

  Serial0.println("Determine zero weight offset");
  //  average 20 measurements.
  myScale.tare(20);
  int32_t offset = myScale.get_offset();

  Serial0.print("OFFSET: ");
  Serial0.println(offset);
  Serial0.println();


  Serial0.println("place a weight on the loadcell");
  //  flush Serial input
  while (Serial0.available()) Serial0.read();

  Serial0.println("enter the weight in (whole) grams and press enter");
  uint32_t weight = 0;
  while (Serial0.peek() != '\n')
  {
    if (Serial0.available())
    {
      char ch = Serial0.read();
      if (isdigit(ch))
      {
        weight *= 10;
        weight = weight + (ch - '0');
      }
    }
  }
  Serial0.print("WEIGHT: ");
  Serial0.println(weight);
  myScale.calibrate_scale(weight, 20);
  float scale = myScale.get_scale();

  Serial0.print("SCALE:  ");
  Serial0.println(scale, 6);

  Serial0.print("\nuse scale.set_offset(");
  Serial0.print(offset);
  Serial0.print("); and scale.set_scale(");
  Serial0.print(scale, 6);
  Serial0.print(");\n");
  Serial0.println("in the setup of your project");

  Serial0.println("\n\n");
}


//  -- END OF FILE --