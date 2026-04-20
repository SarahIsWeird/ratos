target remote localhost:1234
file rats.bin
tui new-layout asmregs { -horizontal asm 1 regs 1 } 1 cmd 1
tui new-layout srcregs { -horizontal src 1 regs 1 } 1 cmd 1
tui layout asmregs
break kmain
continue
disas

