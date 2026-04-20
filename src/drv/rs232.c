#include "drv/rs232.h"

#include "util.h"

static inline int can_tx(const uint16_t port) {
    return inb(S_LINE_STATUS(port)) & S_LSR_TX_REG_EMPTY;
}

static inline int can_rx(const uint16_t port) {
    return inb(S_LINE_STATUS(port)) & S_LSR_DATA_READY;
}

int serial_init(const uint16_t port, const uint16_t baud_rate_divisor) {
    /* disable serial IRQs */
    outb(S_INT_ENABLE(port), 0);

    /* enable Divisor Latch Access, write the baud divisor and disable DLA again */
    outb(S_LINE_CONTROL(port), S_LCR_DLAB);
    outb(S_DIVISOR_LSB(port), (uint8_t) (baud_rate_divisor & 0xff));
    outb(S_DIVISOR_MSB(port), (uint8_t) ((baud_rate_divisor >> 8) & 0xff));

    /* disable DLA, set the serial frame info */
    outb(S_LINE_CONTROL(port), S_LCR_DATA_BITS_8 | S_LCR_PARITY_NONE | S_LCR_STOP_BITS_1);
    
    /* enable and clear FIFOs, set threshold for when interrupts fire */
    outb(S_FIFO_CONTROL(port),
        S_FCR_ENABLE_FIFOS
        | S_FCR_CLEAR_RX_FIFO | S_FCR_CLEAR_TX_FIFO
        | S_FCR_ITL_14_BYTES);
    
    /* enable IRQs, set Data Terminal Ready and Ready To Send */
    outb(S_MODEM_CONTROL(port), S_MCR_OUT2 | S_MCR_DTR | S_MCR_RTS);
    
    /* enable loopback testing */
    outb(S_MODEM_CONTROL(port), S_MCR_OUT2 /* | S_MCR_DTR */ | S_MCR_RTS | S_MCR_LOOPBACK);
    
    /* test the serial port via loopback */
    static const uint8_t TEST_BYTES[4] = { 0x00, 0xff, 0xa5, 0x5a };
    for (size_t i = 0; i < 4; i++) {
        outb(S_TX_RX(port), TEST_BYTES[i]);
        if (inb(S_TX_RX(port)) != TEST_BYTES[i]) return -1;
    }

    /* the serial port seems to work :D */

    /* enable IRQs, set Data Terminal Ready and Ready To Send */
    outb(S_MODEM_CONTROL(port), S_MCR_OUT2 | S_MCR_DTR | S_MCR_RTS);
    return 0;
}

void serial_write(const uint16_t port, const char *str, const size_t size) {
    for (size_t i = 0; ((size != 0) && (i < size)) || ((size == 0) && (str[i] != 0)); i++) {
        if (!can_tx(port)) { /* busy wait */ }
        outb(S_TX_RX(port), (uint8_t) str[i]);
    }
}

size_t serial_read(const uint16_t port, char *buf, const size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (!can_rx(port)) return i;
        buf[i] = (char) inb(S_TX_RX(port));
    }

    return size;
}
