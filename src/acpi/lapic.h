#ifndef LAPIC_H
#define LAPIC_H

#define LAPIC_REG_ID                        0x0020
#define LAPIC_REG_VERSION                   0x0030
#define LAPIC_REG_TASK_PRIORITY             0x0080
#define LAPIC_REG_ARBITRATION_PRIORITY      0x0090
#define LAPIC_REG_PROCESSOR_PRIORITY        0x00a0
#define LAPIC_REG_EOI                       0x00b0
#define LAPIC_REG_REMOTE_READ               0x00c0
#define LAPIC_REG_LOGICAL_DESTINATION       0x00d0
#define LAPIC_REG_DESTINATION_FORMAT        0x00e0
#define LAPIC_REG_SPURIOUS_INTERRUPT_VECTOR 0x00f0
#define LAPIC_REG_ISR_0                     0x0100
#define LAPIC_REG_ISR_1                     0x0110
#define LAPIC_REG_ISR_2                     0x0120
#define LAPIC_REG_ISR_3                     0x0130
#define LAPIC_REG_ISR_4                     0x0140
#define LAPIC_REG_ISR_5                     0x0150
#define LAPIC_REG_ISR_6                     0x0160
#define LAPIC_REG_ISR_7                     0x0170
#define LAPIC_REG_TMR_0                     0x0180
#define LAPIC_REG_TMR_1                     0x0190
#define LAPIC_REG_TMR_2                     0x01a0
#define LAPIC_REG_TMR_3                     0x01b0
#define LAPIC_REG_TMR_4                     0x01c0
#define LAPIC_REG_TMR_5                     0x01d0
#define LAPIC_REG_TMR_6                     0x01e0
#define LAPIC_REG_TMR_7                     0x01f0
#define LAPIC_REG_IRR_0                     0x0200
#define LAPIC_REG_IRR_1                     0x0210
#define LAPIC_REG_IRR_2                     0x0220
#define LAPIC_REG_IRR_3                     0x0230
#define LAPIC_REG_IRR_4                     0x0240
#define LAPIC_REG_IRR_5                     0x0250
#define LAPIC_REG_IRR_6                     0x0260
#define LAPIC_REG_IRR_7                     0x0270
#define LAPIC_REG_ERROR_STATUS              0x0280
#define LAPIC_REG_LVT_CMCI                  0x02f0
#define LAPIC_REG_ICR_0                     0x0300
#define LAPIC_REG_ICR_1                     0x0310
#define LAPIC_REG_LVT_TIMER                 0x0320
#define LAPIC_REG_LVT_THERMAL_SENSOR        0x0330
#define LAPIC_REG_PERF_MONITOR_COUNTERS     0x0340
#define LAPIC_REG_LVT_LINT0                 0x0350
#define LAPIC_REG_LVT_LINT1                 0x0360
#define LAPIC_REG_LVT_ERROR                 0x0370
#define LAPIC_REG_INITIAL_COUNT             0x0380
#define LAPIC_REG_CURRENT_COUNT             0x0390
#define LAPIC_REG_DIVIDE_CONFIGURATION      0x03e0

void lapic_init(void);

#endif /* LAPIC_H */
