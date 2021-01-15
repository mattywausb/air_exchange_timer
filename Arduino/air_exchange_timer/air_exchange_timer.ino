
#include "picturelamp.h"
#include "mainSettings.h"

#define DEBUG_ON

#ifdef TRACE_ON
  #define TRACE_MODES
  #define TRACE_COUNTDOWN
#endif 

#ifndef DEBUG_ON
    #define DEBUG_TIME_SPEED 1
    #define SECONDS_TO_MEASURE 300
    #define LAMP_SWITCH_DURATION 333
#else
    #define DEBUG_TIME_SPEED 4
    #define SECONDS_TO_MEASURE 60
    #define LAMP_SWITCH_DURATION 1000
#endif

#define LAMP_COUNT 3

#define LAMP_FADE_DURATION 1000
#define LAMP_SWITCH_DURATION 333

#define iRED 0
#define iGREEN 1
#define iBLUE 2

#define SECONDS_PER_DAY 86400





struct catalog {
  const int seconds_border;
  const byte color_index_from;
  const byte color_index_to;
} const time_color_map[] = {
  {160,3,3},  // >= 2:40
  {80,3,4},   // >= 1:40 from red to orange
  {40,4,5},   // >= 0:40 from orange to yellow
  {0,5,0}    // >= 0 from yellow to black
};

#define TIME_COLOR_MAP_COUNT 4

unsigned long g_mode_start_time=0;
unsigned long g_mode_last_action_time=0;
int g_time_color_map_position=0;

PictureLamp g_picture_lamp[LAMP_COUNT];
int g_current_lamp_index=0;



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
      Serial.print(F(" bytes free memory. uptime="));
      Serial.println(millis()/1000);
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
      Serial.print(F("TRACE_MODES> #COUNTDOWN_MODE: uptime="));
      Serial.println(millis()/1000);
    #endif
    g_process_mode=COUNTDOWN_MODE;
    input_IgnoreUntilRelease();

    digitalWrite(LED_BUILTIN, false);
    g_mode_start_time=millis();
    g_mode_last_action_time=g_mode_start_time;
    g_time_color_map_position=0;
    g_current_lamp_index=0;

    play_start_countdown_animation();

    //Set all lamps to first color and trigger Fade
    for(int i=0;i<LAMP_COUNT;i++) 
    {
         g_picture_lamp[i].setCurrentColor(time_color_map[g_time_color_map_position].color_index_to);
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

    unsigned long current_time=millis();
    int seconds_left=SECONDS_TO_MEASURE-((current_time-g_mode_start_time)/1000);
    if (seconds_left<=0) {
      enter_WINDOW_OPEN_MODE();
      return;
    }
   
    while(seconds_left<time_color_map[g_time_color_map_position].seconds_border/DEBUG_TIME_SPEED) g_time_color_map_position++;

    if((current_time-g_mode_last_action_time)>LAMP_SWITCH_DURATION) { // Acitvate next lamp
      g_mode_last_action_time=current_time;
      int seconds_left_on_map_position  =seconds_left - time_color_map[g_time_color_map_position].seconds_border/DEBUG_TIME_SPEED;
      int section_length = g_time_color_map_position==0?1000: (time_color_map[g_time_color_map_position-1].seconds_border/DEBUG_TIME_SPEED -time_color_map[g_time_color_map_position].seconds_border/DEBUG_TIME_SPEED)/ LAMP_COUNT;
      #ifdef TRACE_COUNTDOWN
        Serial.print(F("TRACE_COUNTDOWN> seconds_left="));Serial.print(seconds_left);
        Serial.print(F(" seconds_left_on_map_position="));Serial.print(seconds_left_on_map_position);
        Serial.print(F(" section_length="));Serial.print(section_length);
        Serial.print(F(" map_position="));Serial.print(g_time_color_map_position);
        Serial.print(F(" current_lamp="));Serial.print(g_current_lamp_index);
      #endif
      
      if(seconds_left_on_map_position<section_length*g_current_lamp_index) {
         g_picture_lamp[g_current_lamp_index].setCurrentColor(time_color_map[g_time_color_map_position].color_index_to);
         #ifdef TRACE_COUNTDOWN
            Serial.print(F(" TO: ")); Serial.println(time_color_map[g_time_color_map_position].color_index_to);
         #endif 
      }else {
         g_picture_lamp[g_current_lamp_index].setCurrentColor(time_color_map[g_time_color_map_position].color_index_from);
         #ifdef TRACE_COUNTDOWN
            Serial.print(F(" FR: ")); Serial.println(time_color_map[g_time_color_map_position].color_index_from);
         #endif 
      }
      g_picture_lamp[g_current_lamp_index].setTargetColor(0.0,0.0,0.0);
      g_picture_lamp[g_current_lamp_index].startTransition(LAMP_FADE_DURATION);
      if(++g_current_lamp_index>=LAMP_COUNT) { 
        g_current_lamp_index=0;
        #ifdef TRACE_COUNTDOWN
          Serial.println(F("-----"));
        #endif
      }
    }

    // Finally calculate and propagate new lamp values
    for(int i=0;i<LAMP_COUNT;i++) 
    {
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
    g_current_lamp_index=0;
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
      output_setPixelColor(g_current_lamp_index,0,0,0); // Shut down current light
      if(++g_current_lamp_index>=LAMP_COUNT) g_current_lamp_index=0;
    }

    output_setPixelColor(g_current_lamp_index,red,green,blue);
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
    g_current_lamp_index=0;
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
      if (i<=g_current_lamp_index) output_setPixelColor(i,red,green,blue); // Shut down current light
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


