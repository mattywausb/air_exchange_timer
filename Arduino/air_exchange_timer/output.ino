#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#ifdef TRACE_ON
#define TRACE_OUTPUT
//#define TRACE_OUTPUT_HIGH
#endif

//#define MOCKUP_LIGHTS

#define CHAIN_PIN_1 2




#define NUMCHAINS 1
#define NUMPIXELS 3
#define PIXEL_BRIGHTNESS 255
Adafruit_NeoPixel light_chain[1]={ Adafruit_NeoPixel(NUMPIXELS, CHAIN_PIN_1, NEO_RGB + NEO_KHZ400)};

/* This map translates the picture light index in to the physical light index 
   an must be adapted to the current physical setup
 */
byte output_lamp_to_pixel_map[3]={ 0, 1,2 };


void output_setup()
{
  for (int i=0;i<NUMCHAINS;i++) 
  {
    light_chain[i].begin(); 
    light_chain[i].setBrightness(PIXEL_BRIGHTNESS);                                 
  }
}

/* output_setLampColor (Lamp=logical lamp) takes RGB values, from 0,0,0 up to 255,255,255 */
void output_setLampColor(byte lamp_index,int red,int green, int blue)
{
  output_setPixelColor(output_lamp_to_pixel_map[lamp_index], red, green, blue);
}

/* output_setPixelColor takes RGB values, from 0,0,0 up to 255,255,255 */
void output_setPixelColor(byte index,int red,int green, int blue)
{
  byte light_index=index;
  byte chain_index=light_index/NUMPIXELS;
  byte pixel_index=light_index%NUMPIXELS;
  #ifdef MOCKUP_LIGHTS
    if(chain_index<NUMCHAINS && pixel_index<NUMPIXELS)
  #endif  
  light_chain[chain_index].setPixelColor(pixel_index, light_chain[chain_index].Color(red,green,blue));
}

void output_show()
{
  #ifdef TRACE_OUTPUT_HIGH
      Serial.println(F("TRACE_OUTPUT_HIGH>output_show"));
  #endif
  for (int i=0;i<NUMCHAINS;i++) 
  {
    light_chain[i].show();                                  
  }
}



