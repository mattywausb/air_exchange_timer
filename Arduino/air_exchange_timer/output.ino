#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#ifdef TRACE_ON
#define TRACE_OUTPUT
//#define TRACE_OUTPUT_HIGH
#endif

//#define MOCKUP_LIGHTS

#define CHAIN_PIN_1 7


 // MOCKUP PIXEL STRIP definition
 


#define NUMCHAINS 1
#define NUMPIXELS 3
#define PIXEL_BRIGHTNESS 255
Adafruit_NeoPixel light_chain[1]={ Adafruit_NeoPixel(NUMPIXELS, CHAIN_PIN_1, NEO_RGB + NEO_KHZ400)};

/* This map translates the picture light index in to the physical light index 
   an must be adapted to the current physical setup
 */
byte light_index_map[3]={ 0, 1,2 };


void output_setup()
{
  for (int i=0;i<NUMCHAINS;i++) 
  {
    light_chain[i].begin(); 
    light_chain[i].setBrightness(PIXEL_BRIGHTNESS);                                 
  }
}

/* output_setLightColor takes RGB values, from 0,0,0 up to 255,255,255 */
void output_setLightColor(byte index,int red,int green, int blue)
{
  byte light_index=light_index_map[index];
  byte chain_index=light_index/NUMPIXELS;
  byte pixel_index=light_index%NUMPIXELS;
  light_chain[chain_index].setPixelColor(pixel_index, light_chain[chain_index].Color(red,green,blue));
}

/* output_setLightColorUnmapped takes RGB values, from 0,0,0 up to 255,255,255 */
void output_setLightColorUnmapped(byte index,int red,int green, int blue)
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
      Serial.println(F(">output_show"));
  #endif
  for (int i=0;i<NUMCHAINS;i++) 
  {
    light_chain[i].show();                                  
  }
}

void output_play_start_anmiation()
{
    #ifdef TRACE_OUTPUT
      Serial.println(F("#TRACE_OUTPUT: output_play_start_anmiation  ### first draft ###"));
    #endif
    for(lx=0;lx<LAMP_COUNT;lx++) {
      output_setLightColor(lx,100,0,200);
      delay(200);
    }
    delay(500);
}


