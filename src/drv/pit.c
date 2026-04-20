#include "pit.h"

#include "util.h"

#define PIT_CHAN0 0x40
#define PIT_CHAN1 0x41
#define PIT_CHAN2 0x42
#define PIT_MODE  0x43

#define PIT_MODE_CHAN0   0x00
#define PIT_MODE_CHAN1   0x40
#define PIT_MODE_CHAN2   0x80
#define PIT_MODE_RB      0xc0
#define PIT_MODE_LATCH   0x00
#define PIT_MODE_LOW     0x10
#define PIT_MODE_HIGH    0x20
#define PIT_MODE_HILO    0x30
#define PIT_MODE_SQ_WAVE 0x03

void pit_init(void) {
    outb(PIT_MODE,  PIT_MODE_CHAN0 | PIT_MODE_HILO | PIT_MODE_SQ_WAVE);
    outb(PIT_CHAN0, 0x01);
    outb(PIT_CHAN0, 0x00);
}
