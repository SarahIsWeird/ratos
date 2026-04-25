target remote localhost:1234
file rats.bin

so gdb_utils/locals.py

tui new-layout asmregs { -horizontal asm 1 regs 1 } 1 cmd 1
tui new-layout srcregs { -horizontal src 1 regs 1 } 1 cmd 1
tui new-layout srcvars { -horizontal src 1 vars 1 } 1 cmd 1
tui layout srcvars

set output-radix 16

break kmain
break virt_init
continue

