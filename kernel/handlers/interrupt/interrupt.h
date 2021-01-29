#include "../task/schedule/schedule.h"
#include "../../screen/screen.h"
#include "../../drivers/keyboard/keyboard.h"

extern void isrDefaultInt();
extern void isrClockInt();
extern void isrKbdInt();