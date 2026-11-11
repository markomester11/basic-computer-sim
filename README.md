# Basic Computer simulator

A register-transfer-level simulator, written in C, of the 16-bit "Basic Computer" from M. Morris Mano's *Computer System Architecture*. Every register is modelled as an array of bits, and each instruction runs as the textbook's micro-operations, one timing signal (T0–T3) at a time.

## What it models

- **Memory:** 4096 words × 16 bits
- **Registers:** PC (12 bit), MAR (12), MBR (16), AC (16), E, I, the 3-bit opcode and a 2-bit sequence counter. Mano calls MAR and MBR "AR" and "DR".
- **Hardware:** ripple-carry adder, two's complement, a 2-to-4 timing decoder and a 3-to-8 operation decoder
- **Instruction cycles:** fetch, indirect and execute, selected by the F and R flip-flops. The interrupt cycle is not implemented.
- **Instructions:**
  - 7 memory-reference instructions: `AND ADD LDA STA BUN BSA ISZ`, with direct or indirect addressing (append `*` for indirect, e.g. `LDA* 100`)
  - 12 register-reference instructions: `CLA CLE CMA CME CIR CIL INC SPA SNA SZA SZE HLT`

## Build and run

```sh
gcc -Wall -o sim main.c -lm
./sim
```

The simulator reads the program from `fajl.txt` in the current folder, runs it until `HLT`, then prints every non-zero memory word as its address followed by its 16 bits.

## Program format

Each line before `HLT` is an instruction: `address MNEMONIC [operand]`. Each line after it is data: `address value`. All numbers are decimal.

The included `fajl.txt` adds 1 and 3:

```
200 LDA 100
201 ADD 101
202 STA 102
203 HLT
100 1
101 3
```

Output:

```
100 0000000000000001
101 0000000000000011
102 0000000000000100    <- 1 + 3 = 4
200 0010000001100100    <- LDA 100: I = 0, opcode 010, address 000001100100
201 0001000001100101    <- ADD 101: opcode 001
202 0011000001100110    <- STA 102: opcode 011
203 0111000000000001    <- HLT: register-reference (opcode 111), bit 15 set
```

## Notes

- Code comments are in Montenegrin.
- Planned: the interrupt cycle and I/O instructions, unit tests for each instruction, and passing the program file as a command-line argument.
