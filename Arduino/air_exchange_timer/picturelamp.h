#ifndef PICTURELAMP_h
#define PICTURELAMP_h

enum transition_type_t {TT_NONE,TT_ON,TT_OFF,TT_BLEND};

#define PL_COLOR_BLACK 0

#define PL_COLOR_CYAN  3
#define PL_COLOR_GREEN 7
#define PL_COLOR_GREEN_DIMMED 9
#define PL_COLOR_GREEN_MEDIUM 8
#define PL_COLOR_PINK  2
#define PL_COLOR_WHITE 1

class PictureLamp
{
  public:
      PictureLamp();

      /* ---- Operations ---- */
      bool updateOutput(byte light_index);          // Calculate current color and send to logical pixel, return true if transition is finished
      void setCurrentColor(float red, float green, float blue); // Values from 0 to 1
      void setCurrentColor(byte palette_index); // Values from 0 to 1
      void setTargetColor(float red, float green, float blue);  // Values from 0 to 1
      void setTargetColor(byte palette_index); // Values from 0 to 1
      void startTransition(unsigned long duration);   // duration in millis
      void endTransition();                         // Skip left over transition operation to target values

      /* ---- State information ---- */
      bool is_transition_in_progess();
      bool is_transition_finished();
      transition_type_t getTransitionType() {return transition_type;};

  protected:
      unsigned long start_transition_time=0;
      unsigned long transition_duration=0; /* 0 = not in transition */
      int current_red,current_green,current_blue;
      int target_red,target_green,target_blue;
      transition_type_t transition_type;
};



#endif /* End of Header file */
