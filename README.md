# Air Exchange Timer
Small device to display the time until air exchange in a room is complete

This project contains all code and documentation

## Concept
Due to the covi-19 pandemic we needed a timed signal prevent the usage of the bathroom until air exchange is completed.
The usecase comes from the situation, were one person in the house needs to keep quarataine but we ony have one bathroom and since we cannot use masks every time in the bathroom the following procedure was established.

The timer is placed directly outside the bathroom near the door.

### When the quaratined person needs to use the bathroom:
* Leave your room and go to the bathroom with mask
* Lock door and do your duty
* Open Window in the bathroom
* Leave bathroom with mask
* Press the button on the timer for 2 Seconds
* Go Back to your room

### Timer display
The Timer lights and blinks 3 RGB LEDs as follows
* Red more then 2:40 Minutes
* Red mixed with Orange more then 2:20 and 2:00
* Orange more then 1:40 Minutes
* orange Mixed with yellow more then 1:20 and 1:00
* 3 Yellow more then 0:40
* 2 Yellow more then 0:20
* 1 Yellow final 20 Seconds
* 1 Cyan slow blinking
* Off (Ready to start next timer)

Pressing and holding the button during a running timer will start over the whole time interval.
There is no way to shorten the process.

### When anybody else needs to use the bathroom:
* Check and wait until the bathroom timer is done 
* Close window
* Hit the button short

### When anybody else comes by and sees cyan lighting
* Close window
* Hit the button short


