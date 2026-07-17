#ifndef PIT_H
#define PIT_H

#define PIT_BASE_FREQUENCY 1193182
#define PIT_CHANNEL0       0x40
#define PIT_CHANNEL1       0x41
#define PIT_CHANNEL2       0x42
#define PIT_COMMAND        0x43

#define PIT_FREQUENCY     1000              // Hz (1 tick = 1ms)
#define TICKS_PER_SEC     PIT_FREQUENCY
#define SCHEDULER_QUANTUM 100               // ticks per slice (100ms)


void pit_init(unsigned int frequency);
unsigned int get_current_tick(void);
#endif