#include "pit.h"
#include "../lib/io.h"
#include "../kernel/interrupt.h"
#include "../kernel/debug.h"
#include "pic.h"
#include "../kernel/scheduler.h"

static volatile unsigned int ticks = 0;

static void pit_interrupt_handler(struct cpu_state *cpu, struct stack_state *stack, unsigned int interrupt) {
    ticks++;
    if (ticks % TICKS_PER_SEC == 0) { // wait 1 second
        LOG_HEX("tick", ticks);
    }
    pic_acknowledge(interrupt);
    if (stack->cs & 0x3 && ticks % SCHEDULER_QUANTUM == 0) 
        schedule(cpu, stack);
}

/**
 * Configure PIT channel 0 to generate IRQ0 at a given frequency.
 *
 * @param frequency Desired tick frequency in Hz (e.g. 100 for 10ms ticks).
 */
void pit_init(unsigned int frequency) {
    if (frequency == 0 || PIT_BASE_FREQUENCY / frequency > 65535 || frequency > 65535) return;
    unsigned int count = PIT_BASE_FREQUENCY / frequency;
    outb(PIT_COMMAND, 0x36); // 00110110
    outb(PIT_CHANNEL0, count&0xFF);
    outb(PIT_CHANNEL0, (count&0xFF00)>>8);
    register_interrupt_handler(0x20, pit_interrupt_handler);
    return;
}

unsigned int get_current_tick(void) {
    return ticks;
}