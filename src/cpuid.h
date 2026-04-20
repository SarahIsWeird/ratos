#ifndef CPUID_H
#define CPUID_H

#include <stdint.h>

#define CPUID_MAX_LEAF_AND_VENDOR_ID 0x00
#define CPUID_MAX_EXTENDED_LEAF      0x80000000

#define CPUID_ADDRESS_SPACE_INFO     0x80000008
#define CPUID_ASI_EAX_PHYS_ADDR_SIZE_OFFSET          0
#define CPUID_ASI_EAX_LIN_ADDR_SIZE_OFFSET           8
#define CPUID_ASI_EAX_GUEST_PHYS_ADDR_SIZE_OFFSET   16
#define CPUID_ASI_EAX_PHYS_ADDR_SIZE_MASK         0xff
#define CPUID_ASI_EAX_LIN_ADDR_SIZE_MASK          0xff
#define CPUID_ASI_EAX_GUEST_PHYS_ADDR_SIZE_MASK   0xff
#define CPUID_ASI_EBX_WBNOINVD_SUPPORT_OFFSET        9
#define CPUID_ASI_EBX_WBNOINVD_SUPPORT_MASK       0x01

static inline void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    uint64_t rax, rbx, rcx, rdx;
    __asm__ ("cpuid" : "=a" (rax), "=b" (rbx), "=c" (rcx), "=d" (rdx) : "a" (leaf), "c" (subleaf));
    *eax = (uint32_t) rax;
    *ebx = (uint32_t) rbx;
    *ecx = (uint32_t) rcx;
    *edx = (uint32_t) rdx;
}

static inline uint32_t cpuid_eax(uint32_t leaf, uint32_t subleaf) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(leaf, subleaf, &eax, &ebx, &ecx, &edx);
    return eax;
}

static inline uint32_t cpuid_ebx(uint32_t leaf, uint32_t subleaf) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(leaf, subleaf, &eax, &ebx, &ecx, &edx);
    return ebx;
}

static inline uint32_t cpuid_ecx(uint32_t leaf, uint32_t subleaf) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(leaf, subleaf, &eax, &ebx, &ecx, &edx);
    return ecx;
}

static inline uint32_t cpuid_edx(uint32_t leaf, uint32_t subleaf) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(leaf, subleaf, &eax, &ebx, &ecx, &edx);
    return edx;
}

#endif /* CPUID_H */
