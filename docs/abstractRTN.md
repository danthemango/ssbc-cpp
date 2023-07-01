SSBC Version 5 Revision 0 (Abstract RTN)
========================================
SSBC - Simple Stack-Based Computer

Processor State
---------------
|               |                           |
| ------------- | ------------------------- |
| PC<15..0>:    | Program Counter           |
| SP<15..0>:    | Stack Pointer             |
| MR<15..0>:    | Memory Register           |
| Ri<7..0>:     | Register i (0 <= i <= 3)  |
| IR<7..0>:     | Instruction Register      |
| Z: 1-bit      | Zero Flag                 |
| N: 1-bit      | Negative Flag             |
| fault:        | 1-bit Fault Indicator     |
| halt:         | 1-bit Halt Indicator      |
| Reset:        | Rest Signal               |

Main Memory
-----------
- MEM[0..2^16-1]<7..0

Instruction Format
------------------
- opcode<3..0> := IR<3..0>

Operands/Address
----------------
- ii<7..0> := MEM[PC]
- s1<7..0> := MEM[SP+1]
- s2<7..0> := MEM[SP+2]
- ext<15..0> := MEM[PC]#MEM[PC+1]

Program Status Word
-------------------
- PSW<7..0> := Z#N#6@0:

Instruction Interpretation
--------------------------
- Ins_interpretation := (Reset -> (PC <- 0x0: SP <- 0xFFFA: halt <- 0x0:
- Fault <- 0x0: ins_interpretation): (NOT Reset) and (NOT Fault) ->
    (IR <- MEM[PC]; set_fault; (NOT Fault) -> (PC <- PC+1; ins_exe));

Fault Detection
---------------
- Set_fault := fault <- NOT (0 <= opcode <= 0xA)

Instruction Execution
---------------------
Ins_exe := \(

    Noop (:= opcode=0) -> PC <- PC+1:
    Halt (:= opcode=1) -> halt <- 0x1:
    Pushimm (:= opcode=2) -> MEM[SP] <- ii; (SP <- SP-1;:PC <- PC+1):
    Pushext (:= opcode=3) -> MEM[SP] <- MEM[ext];
        (SP <- SP-1: PC <- PC+2):
    Popinh (:= opcode=4) -> SP <- SP+1:
    Popext (:= opcode=5) -> MEM[ext] <- s1; (SP <- SP+1: PC <- PC+2):
    Jnz (:= opcode=6) -> (NOT Z) -> PC <- ext:
    Jnn (:= opcode=7) -> (NOT N) -> PC <- ext:
    Add (:= opcode=8) -> MEM[SP+2] <- s1+s2;
        (Z <- NOT (R2<7> OR R2<6> .. OR R2<0>): N <- R2<7>: SP <- SP+1:
    Sub (:= opcode=9) -> MEM[SP+2] <- s1-s2;
        (Z <- NOT (R2<7> OR R2<6> .. OR R2<0>): N <- R2<7>: SP <- SP+1:
    Nor (:= opcode=10) -> MEM[SP+2] <- s1 NOR s2; SP <-SP+1:

\); ins_interpretation):

Memory Map
----------
- PSW mapped to 0xFFFB
- Port A (read only)  mapped to 0xFFFC
- Port B (write only) mapped to 0xFFFD
- Port C (read only)  mapped to 0xFFFE
- Port D (write only) mapped to 0xFFFF

Notation
--------
- -> if-then: true condition on left yields value and/or
- \- Action on right
- <- Register transfer: register on LHS stores value from RHS
- : Parallel Seperator: actions or evaluations carried out
simultaneously
- ; Sequential Seperator: RHS evaluated and/or performed
after LHS
- := Definition: text substitution
- <7..0> 8 bits, labelled from bit 7 to 0
