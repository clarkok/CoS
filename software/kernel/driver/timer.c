#include "timer.h"

#include "proc/proc.h"

#define TIMER_ADDR          0xFFFFFE04u
#define TIMER_COUNT_PER_MS  100000u

#define TIMER_NEW_VALUE     (TIMER_COUNT_PER_MS * TIMER_INTERVAL)

static inline void
out_timer(size_t timer)
{
    volatile size_t *timer_ptr = (volatile size_t*)(TIMER_ADDR);
    *timer_ptr = timer;
}

void
timer_handler()
{
    proc_schedule();

    out_timer(TIMER_NEW_VALUE);
}

void
timer_init()
{
    dbg_uart_str("Timer init\n");
    dbg_uart_hex((size_t)timer_handler);
    INTERRUPT_HANDLER_TABLE[INT_TIMER] = (size_t)(&timer_handler);
    enable_int(INT_TIMER);
}
