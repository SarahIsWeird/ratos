#ifndef RS232_H
#define RS232_H

#include <stdint.h>
#include <stddef.h>

typedef uint16_t serial_port_t;

#define S_COM1                   ((serial_port_t) 0x3f8)
#define S_COM2                   ((serial_port_t) 0x2f8)

#define KERNEL_LOGGING_PORT      S_COM1

#define S_BAUD_115200            ((uint16_t)  1)
#define S_BAUD_57600             ((uint16_t)  2)
#define S_BAUD_38400             ((uint16_t)  3)
#define S_BAUD_28800             ((uint16_t)  4)
#define S_BAUD_19200             ((uint16_t)  5)
#define S_BAUD_9600              ((uint16_t)  6)
#define S_BAUD_4800              ((uint16_t)  7)
#define S_BAUD_2400              ((uint16_t)  8)
#define S_BAUD_1800              ((uint16_t)  9)
#define S_BAUD_1200              ((uint16_t) 10)
#define S_BAUD_600               ((uint16_t) 11)
#define S_BAUD_300               ((uint16_t) 12)

#define S_TX_RX(port)            ((port) + 0)
#define S_DIVISOR_LSB(port)      ((port) + 0)
#define S_INT_ENABLE(port)       ((port) + 1)
#define S_DIVISOR_MSB(port)      ((port) + 1)
#define S_INTERRUPT_IDENT(port)  ((port) + 2)
#define S_FIFO_CONTROL(port)     ((port) + 2)
#define S_LINE_CONTROL(port)     ((port) + 3)
#define S_MODEM_CONTROL(port)    ((port) + 4)
#define S_LINE_STATUS(port)      ((port) + 5)
#define S_MODEM_STATUS(port)     ((port) + 6)
#define S_SCRATCH_REG(port)      ((port) + 7)

#define S_LCR_DLAB               ((uint8_t) 0b10000000)
#define S_LCR_BREAK_EN           ((uint8_t) 0x01000000)
#define S_LCR_PARITY_NONE        ((uint8_t) 0x00000000)
#define S_LCR_PARITY_ODD         ((uint8_t) 0x00001000)
#define S_LCR_PARITY_EVEN        ((uint8_t) 0b00011000)
#define S_LCR_PARITY_MARK        ((uint8_t) 0b00101000)
#define S_LCR_PARITY_SPACE       ((uint8_t) 0b00111000)
#define S_LCR_STOP_BITS_1        ((uint8_t) 0b00000000)
#define S_LCR_STOP_BITS_1_5      ((uint8_t) 0b00000100)
#define S_LCR_STOP_BITS_2        ((uint8_t) 0b00000100)
#define S_LCR_DATA_BITS_5        ((uint8_t) 0b00000000)
#define S_LCR_DATA_BITS_6        ((uint8_t) 0b00000001)
#define S_LCR_DATA_BITS_7        ((uint8_t) 0b00000010)
#define S_LCR_DATA_BITS_8        ((uint8_t) 0b00000011)

#define S_IER_MODEM_STATUS       ((uint8_t) 0b00001000)
#define S_IER_REC_LINE_STATUS    ((uint8_t) 0b00000100)
#define S_IER_TX_REG_EMPTY       ((uint8_t) 0b00000010)
#define S_IER_RX_DATA_AVAIL      ((uint8_t) 0b00000001)

#define S_FCR_ITL_1_BYTE         ((uint8_t) 0b00000000)
#define S_FCR_ITL_4_BYTES        ((uint8_t) 0b01000000)
#define S_FCR_ITL_8_BYTES        ((uint8_t) 0b10000000)
#define S_FCR_ITL_14_BYTES       ((uint8_t) 0b11000000)
#define S_FCR_DMA_MODE_SELECT    ((uint8_t) 0b00001000)
#define S_FCR_CLEAR_TX_FIFO      ((uint8_t) 0b00000100)
#define S_FCR_CLEAR_RX_FIFO      ((uint8_t) 0b00000010)
#define S_FCR_ENABLE_FIFOS       ((uint8_t) 0b00000001)

#define S_IIR_FIFO_MASK          ((uint8_t) 0b11000000)
#define S_IIR_FIFO_DISABLED      ((uint8_t) 0b00000000)
#define S_IIR_FIFO_UNUSABLE      ((uint8_t) 0b01000000)
#define S_IIR_FIFO_ENABLED       ((uint8_t) 0b10000000)
#define S_IIR_TIMEOUT_PENDING    ((uint8_t) 0b00001000)
#define S_IIR_INT_MASK           ((uint8_t) 0b00000110)
#define S_IIR_INT_MODEM_STATUS   ((uint8_t) 0b00000000)
#define S_IIR_INT_TX_REG_EMPTY   ((uint8_t) 0b00000010)
#define S_IIR_INT_RX_DATA_AVAIL  ((uint8_t) 0b00000100)
#define S_IIR_INT_RX_LINE_STATUS ((uint8_t) 0b00000110)
#define S_IIR_PENDING_MASK       ((uint8_t) 0b00000001)
#define S_IIR_PENDING            ((uint8_t) 0b00000000)

#define S_MCR_LOOPBACK           ((uint8_t) (1 << 4))
#define S_MCR_OUT2               ((uint8_t) (1 << 3))
#define S_MCR_OUT1               ((uint8_t) (1 << 2))
#define S_MCR_RTS                ((uint8_t) (1 << 1))
#define S_MCR_DTR                ((uint8_t) (1 << 0))

#define S_LSR_IMPENDING_ERROR    ((uint8_t) (1 << 7))
#define S_LSR_TX_EMPTY           ((uint8_t) (1 << 6))
#define S_LSR_TX_REG_EMPTY       ((uint8_t) (1 << 5))
#define S_LSR_BREAK_INDICATOR    ((uint8_t) (1 << 4))
#define S_LSR_FRAMING_ERROR      ((uint8_t) (1 << 3))
#define S_LSR_PARITY_ERROR       ((uint8_t) (1 << 2))
#define S_LSR_OVERRUN_ERROR      ((uint8_t) (1 << 1))
#define S_LSR_DATA_READY         ((uint8_t) (1 << 0))

int serial_init(serial_port_t port, uint16_t baud_rate);

/**
 * Writes a string of characters to the specified serial port.
 * @param size The size of the string to write. Can be `0`, in which case the string is written
 *             until a NULL terminator is found.
 */
void serial_write(serial_port_t port, const char *str, size_t size);

/**
 * Reads a string of characters from the specified serial port.
 * @param buf The buffer to write the characters into. Must be preallocated for the given size.
 * @param size The maximum number of characters to read.
 * @returns The number of bytes read (might be 0).
 */
size_t serial_read(serial_port_t port, char *buf, size_t size);

#endif
