#ifndef PIT_H
#define PIT_H

#define PIT_BASE_FREQUENCY 1193182
#define PIT_CHANNEL0       0x40
#define PIT_CHANNEL1       0x41
#define PIT_CHANNEL2       0x42
#define PIT_COMMAND        0x43

void pit_init(unsigned int frequency);

#endif