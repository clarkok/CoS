#include "driver.h"

#include "timer.h"
#include "disk.h"

void
driver_init()
{
    timer_init();
    disk_init();
}
