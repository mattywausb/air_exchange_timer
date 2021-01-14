
#include "picturelamp.h"
#include "mainSettings.h"


//#define DEBUG_ON

#ifdef TRACE_ON
//#define TRACE_LOGIC

#define TRACE_MODES
//#define TRACE_TIMING
#define TRACE_CLOCK
#define TRACE_CLOCK_TIME
#endif 

#define LAMP_COUNT 3

#defin LAMP_FADE_DURATION 1000

#define iRED 0
#define iGREEN 1
#define iBLUE 2

#define SECONDS_PER_DAY 86400

float g_color_palette[][3]={
          {0  ,0  ,0  },  //0 = black
          {1  ,0.7,0  },  //1 = yellow
          {0  ,0.8  ,0.8  },  //2 = cyan
          {0  ,0.5,0.08}, //3 = mid green
          {0.2,0.1,0 },  //4 = dark brown
          {1,0.0,0  },  //5 = red
          {0  ,0  ,0.8},  //6 = blue
          {1  ,1  ,1  },  //7 = white
          {0.8  ,0  ,0.8  },  //8 = pink
          {1  ,0.3,0  },  //9 = orange
          {0  ,1  ,0  },  // 10 =bright green
          {0.1  ,0  ,0.75  }  //11 = dark purple
};

unsinged long g_mode_start_time=0;
unsinged long g_mode_last_action_time=0;


/* Control */
enum PROCESS_MODES {
  IDLE_MODE, 
  COUNTDOWN_MODE,
  WINDOW_OPEN_MODE,
  TEST_MODE_FADE_SOLO,
  TEST_MODE_SCALING,
};

PROCESS_MODES g_process_mode = IDLE_MODEODE; 

/* **************** setup ******************************* */

void setup() {
  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, false);
  output_setup();
  input_setup();
  delay(700); // wait for chains to power up completetly

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

/* ========= SHOW_MODE ======== */
void enter_IDLE_MODE()
{
    #ifdef TRACE_MODES
      Serial.print(F("#IDLE_MODE: "));
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
    if(input_button_0_GotReleased()) 
    {
      if(input_getLastPressDuration()>1500)   // Release after Long press
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
      Serial.print(F("#COUNTDOWN_MODE:start "));
      Serial.println(millis()/1000);
    #endif
    g_process_mode=COUNTDOWN_MODE;
    input_IgnoreUntilRelease();

    digitalWrite(LED_BUILTIN, false);
    g_mode_start_time=millis();
    g_mode_last_action_time=g_mode_start_time;

    output_play_start_animation();
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
    if(input_button_0_GotReleased()) 
    {
       if(input_getLastPressDuration()>10000)   // Release after 10 second press
       {
        enter_TEST_MODE_FADE_SOLO();
        return;
       }
      if(input_getLastPressDuration()>1500)   // Release after Long press
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
      Serial.println(F("#WINDOW_OPEN_MODE"));
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
    if(input_button_0_GotReleased()) 
    {
      if(input_getLastPressDuration()>1500)   // Release after Long press
        enter_COUNTDOWN_MODE();
      else
        enter_IDLE_MODE();
      return;
    }
} 
   

/* ========= TEST_MODE_FADE_SOLO ======== */

void enter_TEST_MODE_FADE_SOLO() 
{
    #ifdef TRACE_MODES
      Serial.println(F("#TEST_MODE_FADE_SOLO"));
    #endif
    g_process_mode=TEST_MODE_FADE_SOLO;
    input_IgnoreUntilRelease();
    digitalWrite(LED_BUILTIN, false);
    g_pic_index=0;
    for(int i=0;i<LAMP_COUNT;i++)  output_setLightColorUnmapped(i,0,0,0);  // shut down all lights
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
      output_setLightColorUnmapped(g_pic_index,0,0,0); // Shut down current light
      if(++g_pic_index>=LAMP_COUNT) g_pic_index=0;
    }

    output_setLightColorUnmapped(g_pic_index,red,green,blue);
    output_show();
}


/* ========= TEST_MODE_SCALING ======== */

void enter_TEST_MODE_SCALING() 
{
    #ifdef TRACE_MODES
      Serial.println(F("#TEST_MODE_SCALING"));
    #endif
    g_process_mode=TEST_MODE_SCALING;
    input_IgnoreUntilRelease();
    digitalWrite(LED_BUILTIN, false);
    g_pic_index=0;
    for(int i=0;i<LAMP_COUNT;i++)  output_setLightColorUnmapped(i,0,0,0);  // shut down all lights
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
      if (i<=g_pic_index) output_setLightColorUnmapped(i,red,green,blue); // Shut down current light
      else output_setLightColorUnmapped(i,0,0,0); 
    }
    output_show();
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


