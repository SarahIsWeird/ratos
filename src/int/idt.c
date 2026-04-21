#include "idt.h"

#include "util.h"
#include "drv/term.h"

#define PIC1_ADDR 0x20
#define PIC2_ADDR 0xa0

#define PIC1_CMD  PIC1_ADDR
#define PIC1_DATA (PIC1_CMD + 1)
#define PIC2_CMD  PIC2_ADDR
#define PIC2_DATA (PIC2_CMD + 1)

#define PIC_EOI         0x20
#define PIC_ICW1_ICW4   0x01
#define PIC_ICW1_INIT   0x10
#define PIC_ICW4_8086   0x01
#define PIC_CASCADE_ID  2
#define PIC_IRQ_MASK(n) (1 << (n))
#define PIC_READ_IRR    0x0a
#define PIC_READ_ISR    0x0b

#define IRQ_FIRST      0x20
#define PIC1_IRQ_START 0x20
#define PIC2_IRQ_START (PIC1_IRQ_START + 0x08)
#define IRQ_LAST       0x2f

extern void idt_load(void);

static void s_pic_init(void) {
    // PIC1: IRQs start at 0x20, cascaded at IRQ2, 8086 mode
    outb(PIC1_CMD,  PIC_ICW1_INIT | PIC_ICW1_ICW4);
    outb(PIC1_DATA, PIC1_IRQ_START);
    outb(PIC1_DATA, PIC_IRQ_MASK(PIC_CASCADE_ID));
    outb(PIC1_DATA, PIC_ICW4_8086);

    // PIC2: IRQs start at 0x28, cascaded to IRQ2, 8086 mode
    outb(PIC2_CMD,  PIC_ICW1_INIT | PIC_ICW1_ICW4);
    outb(PIC2_DATA, PIC1_IRQ_START);
    outb(PIC2_DATA, PIC_CASCADE_ID);
    outb(PIC2_DATA, PIC_ICW4_8086);

    // Mask IRQs
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

static uint16_t s_pic_read_regs(uint8_t reg) {
    outb(PIC1_CMD, reg);
    outb(PIC2_CMD, reg);

    return inb(PIC2_CMD) << 8 | inb(PIC1_CMD);
}

static uint16_t s_pic_read_irr(void) {
    return s_pic_read_regs(PIC_READ_IRR);
}

static uint16_t s_pic_read_isr(void) {
    return s_pic_read_regs(PIC_READ_ISR);
}

static state_t *s_handle_spurious_interrupt(state_t *state) {
    if (state->int_no >= PIC2_IRQ_START) {
        outb(PIC1_CMD, PIC_EOI);
    }

    kwarn("Spurious interrupt on IRQ %d!\n", state->int_no - IRQ_FIRST);
    return state;
}

static state_t *s_handle_irq(state_t *state) {
    const unsigned int irq_no = (unsigned int) (state->int_no - IRQ_FIRST);
    if (!(s_pic_read_isr() & PIC_IRQ_MASK(irq_no))) {
        return s_handle_spurious_interrupt(state);
    }

    kdebug("irq %d\n", state->int_no);
    
    if (state->int_no >= PIC2_IRQ_START) {
        outb(PIC2_CMD, PIC_EOI);
    }

    outb(PIC1_CMD, PIC_EOI);

    return state;
}

static void s_print_regs(state_t *state) {
    kerror("int/exception 0x%02llx, error code %#0p\n", state->int_no, state->err_no);
    if (state->int_no == 0x0e) {
        uint64_t cr2;
        __asm__ ("mov %%cr2, %0" : "=r" (cr2));
        kerror("cr2: %0p\n", cr2);
    }

    kerror("rax: %0p, rbx: %0p, rcx: %0p, rdx: %0p,\n", state->rax, state->rbx, state->rcx, state->rdx);
    kerror("rdi: %0p, rsi: %0p, rsp: %0p, rbp: %0p,\n", state->rdi, state->rsi, state->rsp, state->rbp);
    kerror("rip: %0p,    cs: 0x%02x, ss: 0x%02x,\n", state->rip, state->cs, state->ss);
    kerror("rflags: %0p\n", state->rflags);
    uint64_t cr0;
    __asm__ ("mov %%cr0, %0" : "=r" (cr0));
    kerror("cr0: %0p\n", cr0);
}

void idt_init(void) {
    idt_load();

    s_pic_init();
}

state_t *idt_int_handler(state_t *state) {
    if (state->int_no >= IRQ_FIRST && state->int_no <= IRQ_LAST) {
        return s_handle_irq(state);
    }

    s_print_regs(state);
    hcf();

    return state;
}
