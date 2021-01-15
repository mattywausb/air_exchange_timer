
#include "picturelamp.h"
#include "mainSettings.h"

#define DEBUG_ON

#ifdef TRACE_ON
  //#define TRACE_LOGIC
  #define TRACE_MODES
  //#define TRACE_TIMING
  #define TRACE_CLOCK
  #define TRACE_CLOCK_TIME
#endif 

#ifndef DEBUG_ON
    #define DEBUG_REDUCTION 1
#else
    #define DEBUG_REDUCTION 4
#endif

#define LAMP_COUNT 3

#define LAMP_FADE_DURATION 1000

#define iRED 0
#define iGREEN 1
#define iBLUE 2

#define SECONDS_PER_DAY 86400



#define SECONDS_TO_MEASURE 300

struct catalog {
  const int seconds_left;
  const byte color_index_from;
  const byte color_index_to;
} const time_color_map[] = {
  {160,3,3},  // >= 2:40
  {80,3,4},   // >= 1:40 from red to orange
  {40,4,5},   // >= 0:40 from orange to yellow
  {40,5,0}    // >= 0 from yellow to black
};

#define TIME_COLOR_MAP_COUNT 4

unsigned long g_mode_start_time=0;
unsigned long g_mode_last_action_time=0;

PictureLamp g_picture_lamp[LAMP_COUNT];
int g_pic_index=0;



/* Control */
enum PROCESS_MODES {
  IDLE_MODE, 
  COUNTDOWN_MODE,
  WINDOW_OPEN_MODE,
  TEST_MODE_FADE_SOLO,
  TEST_MODE_SCALING,
};

PROCESS_MODES g_process_mode = IDLE_MODE; 

/* **************** setup ******************************* */

void setup() {
  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
    #ifdef DEBUG_ON
          Serial.println(F("TRACE_ON> !!! DEBUG MODE IS ACTIVE !!!"));
    #endif
  #endif
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, false);
  output_setup();
  input_setup();
  delay(700); // wait for chains to power up completetly

  play_power_on_animation();
  enter_IDLE_MODE();
}

/* **************** LOOP ******************************* */
void loop() 
{
   input_switches_scan_tick();

   switch(g_process_mode) {
    case IDLE_MODE: process_IDLE_MODE();break;
    case COUNTDOWN_MODE:process_COUNTDOWN_MODE();break;
    case WINDOW_OPEN_MODE:process_WINDOW_OPEN_MODE();break;
    case TEST_MODE_FADE_SOLO:process_TEST_MODE_FADE_SOLO();break;
    case TEST_MODE_SCALING: process_TEST_MODE_SCALING();break;
   } // switch
}

/* ========= IDLE_MODE ======== */
void enter_IDLE_MODE()
{
    #ifdef TRACE_MODES
      Serial.print(F("TRACE_MODES> #IDLE_MODE: "));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
    #endif
    g_process_mode=IDLE_MODE;
    input_IgnoreUntilRelease();
    digitalWrite(LED_BUILTIN, false);
    
    for(int i=0;i<LAMP_COUNT;i++) 
      {
         g_picture_lamp[i].setCurrentColor(0.0,0.0,0.0); // All Lamps off
         g_picture_lamp[i].updateOutput(i);
       }
    output_show();

    g_mode_start_time=millis();
    g_mode_last_action_time=g_mode_start_time;
}

void process_IDLE_MODE()
{
    if(input_button_0_IsPressed()) 
    {
      if(input_getCurrentPressDuration()>1500)   // Long press
        enter_COUNTDOWN_MODE();
      return;
    }
    
    unsigned long current_time=millis();
    
    if(current_time-g_mode_last_action_time>1000) {
      g_mode_last_action_time=current_time;
      digitalWrite(LED_BUILTIN, (current_time/1000)%2);
    }
}

/* ========= COUNTDOWN_MODE ======== */

void enter_COUNTDOWN_MODE()
{
    #ifdef TRACE_MODES
      Serial.print(F("TRACE_MODES> #COUNTDOWN_MODE: second="));
      Serial.println(millis()/1000);
    #endif
    g_process_mode=COUNTDOWN_MODE;
    input_IgnoreUntilRelease();

    digitalWrite(LED_BUILTIN, false);
    g_mode_start_time=millis();
    g_mode_last_action_time=g_mode_start_time;

    play_start_countdown_animation();
    for(int i=0;i<LAMP_COUNT;i++) 
    {
         g_picture_lamp[i].setCurrentColor(1.0,0.0,0.0); // red
         g_picture_lamp[i].setTargetColor(0.0,0.0,0.0);
         g_picture_lamp[i].startTransition(LAMP_FADE_DURATION);
         g_picture_lamp[i].updateOutput(i);
    }
    output_show();
}

void process_COUNTDOWN_MODE()
{
    if(input_button_0_IsPressed()) 
    {
      if(input_getCurrentPressDuration()>1500)   // Long press
        enter_COUNTDOWN_MODE();
      return;
    }

    // Finally calculate and propagate new lamp values
    for(int i=0;i<LAMP_COUNT;i++) 
    {
         g_picture_lamp[i].setCurrentColor(1.0,0.0,0.0); // red
         g_picture_lamp[i].setTargetColor(0.0,0.0,0.0);
         g_picture_lamp[i].startTransition(LAMP_FADE_DURATION);
         g_picture_lamp[i].updateOutput(i);
    }
    output_show();
}
 

/* ========= WINDOW_OPEN_MODE ======== */

void enter_WINDOW_OPEN_MODE() 
{
    #ifdef TRACE_MODES
      Serial.println(F("TRACE_MODES> #WINDOW_OPEN_MODE"));
    #endif
    g_process_mode=WINDOW_OPEN_MODE;
    digitalWrite(LED_BUILTIN, false);

    g_mode_start_time=millis();
    g_mode_last_action_time=g_mode_start_time;
    
    for(int i=0;i<LAMP_COUNT;i++)  /* Initialize all lamps */
    {
         g_picture_lamp[i].setTargetColor(0,1,1);
         g_picture_lamp[i].endTransition();
         g_picture_lamp[i].updateOutput(i);
    }
    output_show();
}

void process_WINDOW_OPEN_MODE()
{
    if(input_button_0_IsPressed()) 
    {
      if(input_getCurrentPressDuration()>1500)   // Long press
        enter_COUNTDOWN_MODE();
      return;
    }
    if(input_button_0_GotReleased()) 
    {
      if(input_getLastPressDuration()<=1500)   // Release after Long press
        enter_IDLE_MODE();
      return;
    }
} 
   

/* ========= TEST_MODE_FADE_SOLO ======== */

void enter_TEST_MODE_FADE_SOLO() 
{
    #ifdef TRACE_MODES
      Serial.println(F("TRACE_MODES> #TEST_MODE_FADE_SOLO"));
    #endif
    g_process_mode=TEST_MODE_FADE_SOLO;
    input_IgnoreUntilRelease();
    digitalWrite(LED_BUILTIN, false);
    g_pic_index=0;
    for(int i=0;i<LAMP_COUNT;i++)  output_setPixelColor(i,0,0,0);  // shut down all lights
    output_show();
}

void process_TEST_MODE_FADE_SOLO()
{
    unsigned long current_time=millis();
    byte red=0;
    byte green=(current_time/10)%255;
    byte blue=0;

    if(input_button_0_GotReleased()) 
    {
      if(input_getLastPressDuration()>1500)   // Release after Long press
      {       
        enter_TEST_MODE_SCALING();
        return;
      }
      output_setPixelColor(g_pic_index,0,0,0); // Shut down current light
      if(++g_pic_index>=LAMP_COUNT) g_pic_index=0;
    }

    output_setPixelColor(g_pic_index,red,green,blue);
    output_show();
}


/* ========= TEST_MODE_SCALING ======== */

void enter_TEST_MODE_SCALING() 
{
    #ifdef TRACE_MODES
      Serial.println(F("TRACE_MODES> #TEST_MODE_SCALING"));
    #endif
    g_process_mode=TEST_MODE_SCALING;
    input_IgnoreUntilRelease();
    digitalWrite(LED_BUILTIN, false);
    g_pic_index=0;
    for(int i=0;i<LAMP_COUNT;i++)  output_setPixelColor(i,0,0,0);  // shut down all lights
    output_show();
}

void process_TEST_MODE_SCALING()
{
    unsigned long current_time=millis();
    byte red=(current_time/10)%255;
    byte green=(current_time/10)%255;
    byte blue=(current_time/10)%255;

     if(input_button_0_GotReleased()) {
        if(input_getLastPressDuration()>5000)   // Release after Long press
        {       
          enter_IDLE_MODE();
          return;
        }
        if(input_getLastPressDuration()>1500)   // Release after Long press
        {       
          enter_TEST_MODE_FADE_SOLO();
          return;
        }
    }
    
    for(int i=0;i<LAMP_COUNT;i++) {
      if (i<=g_pic_index) output_setPixelColor(i,red,green,blue); // Shut down current light
      else output_setPixelColor(i,0,0,0); 
    }
    output_show();
}


/* ******************** Fix Animations  *************** */
void play_power_on_animation(){
    #ifdef TRACE_ON
      Serial.println(F("TRACE_ON> play_power_on_animation  ### first draft ###"));
    #endif
    for(int lmp=0;lmp<LAMP_COUNT;lmp++) {
      output_setLampColor(lmp,100,100,200);
      delay(200);
    }
    delay(500);
}

void play_start_countdown_animation()
{
    #ifdef TRACE_OUTPUT
      Serial.println(F("TRACE_ON> output_play_start_anmiation  ### first draft ###"));
    #endif
    for(int lmp=0;lmp<LAMP_COUNT;lmp++) {
      output_setLampColor(lmp,100,0,200);
      delay(200);
    }
    delay(500);
}


/* ******************** Memory Helper *************** */
 
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}


