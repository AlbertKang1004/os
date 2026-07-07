#include "pit.h"
#include "../lib/io.h"

/**
 * Configure PIT channel 0 to generate IRQ0 at a given frequency.
 *
 * @param frequency Desired tick frequency in Hz (e.g. 100 for 10ms ticks).
 */
void pit_init(unsigned int frequency) {
    if (frequency == 0 || frequency > 65535) return;
    unsigned int count = PIT_BASE_FREQUENCY / frequency;
    outb(PIT_COMMAND, 0x36); // 00110110
    outb(PIT_CHANNEL0, count&0xFF);
    outb(PIT_CHANNEL0, (count&0xFF00)>>8);
    return;
}