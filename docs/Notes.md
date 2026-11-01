# Host Architecture & System Philosophy

- **Host Memory Allocation** The emulator allocates no heap memory; the entire 64 KB RAM is allocated statically on the host machine's stack.
- **Hardware Interfacing Principle** The CPU reads and writes its internal registers and memory autonomously. However, without I/O, I will never be able to see any output from this CPU. Interfacing with peripherals is what brings the hardware to life.
- **System Dependency** The 6502 **cannot operate without memory**. To execute any instructions, the CPU minimum requirements include:
  - **Zero Page (`$0000 - $00FF`)**: Most core instructions directly rely on it for operands and addressing.
  - **Stack Pointer (SP)**: Must point to a valid memory region to handle stack operations and execution context.
  - **Stack Memory (`$0100 - $01FF`)**: Physical memory area required by the **Stack Pointer (SP)** to execute subroutine calls (`JSR`) and stack operations.
  - **Program & User RAM (`$0200+`)**: Additional memory required to load actual code and user data.

# Architecture(As a little-endian)

**Register**: A small, high-speed memory storage area directly accessible by the processor.

**Zero Page(`$0000 - $00FF`)**: The first 256 bytes of memory.

- Special addressing modes across most instructions allow accessing this page **one cycle faster** than elsewhere.

- Functions effectively as a set of additional general-purpose registers to minimize clock cycle consumption.

**Stack Memory**: The second 256 bytes of memory reserved for hardware stack operations.

**Reserved Memory Addresses**:

- **Reset Vector**:  The hardcoded memory location read by the CPU upon startup or hard reset.

**Memory-Mapped I/O**:

- Hardware devices are mapped to memory regions to exchange data.
- Certain bits in the 64 KB RAM are reserved for hardware and not accessible for general memory.

# Registers

**Program Counter (PC)**:

-  A pointer pointing to the next instruction to be fetched.

**Stack Pointer (SP)**:

- The address of the current part of the stack being used.
- **Stack**: A reusable memory region where data is dynamically pushed and popped during execution.

**Processor Status**:

- A single byte containing individual flags set depending on instruction outcomes or hardware events (e.g., IRQ - Interrupt Request).

![Processor Flags](F:\6502-Emulator-C-20\Processor Flags.png)

# Mechanisms

**Reset Mechanism**  The CPU requires a deterministic hardware mechanism to reset itself into a known valid state upon booting or hard reset.  

* **Implementation**:    
  1. Locate the hardcoded Reset Vector at memory address `$FFFC`.    
  2. Jump to this target address to begin executing the kernel/boot routine in ROM.

**Executing Instructions:**

1. Needed Info:

- CPU Clock: On every clock tick, the CPU can either fetch one byte from memory or execute an instruction.

2. Implementation

​		(1) Pass in `Memory` (where instructions and data reside) and the requested execution cycle count(for I may want CPU to execute a certain number of instructions/stop while other things are happening).

​		(2) Run an execution loop, decrementing the cycle counter until no cycles remain, then return.

​		(3) Fetch the next instruction byte (Opcode) from memory via `FetchByte()`.

​		(4) `switch` on the fetched Opcode.

​		(5) Do the operation stated on the instruction.

**Fetching Instructions:**

Fetch the next thing that is pointed to by the program counter.

1. Needed info:

   Number of cycles, memory.

2.  Implementation

- Read a byte from memory at the address stored in PC.
- Increment the program counter (`PC++`) (every time I fetch, the program counter moves to the next step).
- Decrement the CPU cycle count by 1.
- Return the fetched byte.

# Load And Store Operations

**LDA(Load Accumulator):** 

Take a byte of data, and put it into the accumulator.

Flag Set: ZERO and NEGATIVE as appropriate.

![LDA OpCode Table](F:\6502-Emulator-C-20\LDA OpCode Table.png)

**Addressing Mode**: The mechanism used by an instruction to locate its operand in memory(Different ways to call an instruction).

**Opcode**: The single-byte machine code fetched from memory that identifies the instruction(The code that the machine actually reads).

**Length in Bytes**: How many bytes the total instruction needs to take up (e.g., LDA Immediate takes 2 bytes: 1 for the Opcode, 1 for the immediate data).

**Addressing Mode :Immediate**

**Cycle Count (2 Cycles)**: The 6502 is an 8-bit processor; each clock cycle can only access the 8-bit data bus once.

**Cycle Breakdown**:

- **Cycle 1**: Read the 1-byte instruction.
- **Cycle 2**: Read the 1-byte immediate value inline from the machine code.

(Immediate: A literal data byte embedded directly in-line within the machine code that I want to load into the accumulator)

**Addressing Mode: Zeropage** 

The next byte after the opcode is the address in zero page.

**Cycle Breakdown (3 Cycles)**:

1. **Cycle 1**: Fetch the Opcode via `FetchByte()`.
2. **Cycle 2**: Fetch the 1-byte Zero Page address stored in machine code.
3. **Cycle 3**: Read the target data byte from the fetched Zero Page address via `ReadByte()`.

**Implementation:**

- **`FetchByte()` vs. `ReadByte()`**:

  - `FetchByte()` reads from memory and increments `PC` (used for fetching instructions/operands from the code stream).
  - `ReadByte()` reads from a target memory address without incrementing `PC` (used for standard memory access).

  **Execution Flow**:

  1. Fetch the Zero Page target address from memory(`PC++`).
  2. Read the byte from the calculated Zero Page address (preserving `PC`).
  3. Load the fetched byte into the accumulator (`A`).
  4. Update processor status flags (`Z` and `N`).

**Addressing Mode: Zeropage X**

The address to be accessed by an instruction is indexed using zero page.

**Implementation:**

* Take the zero page address from the instruction. 
* Add the current value of the X register to it. 

**Cycles:** 

* 1 cycle: Fetch the instruction. 
* 1 cycle: Fetch the zeropage address stored in machine code. 
* 1 cycle: Add the X value onto the zeropage address. 
* 1 cycle: Read the byte.

**Note:** The address calculation wraps around if the sum of the base address and the register exceed $FF.

**Addressing Mode: Absolute X**

```
memory[base_address + X]` or `base_address[X]
```

The CPU takes a full 16-bit base address from the instruction and adds the current value of the **X register** to find the final target address in memory. 

**Page Crossing**

A 16-bit address is split into two halves: **Low Byte** and **High Byte**.

**Step 1 (Low Byte Addition):**

- The CPU adds the 8-bit `X` register directly to the **Low Byte**.
- If the result is 255 or less, it stays within the current page. The High Byte is already valid, so the operation completes in **4 cycles**.

**Step 2 (Page Boundary Crossed):**

- If the addition of the X register creates a value bigger than 255, **the target address crosses the page boundary**.
- An internal carry notifies the CPU: *"A value bigger than 255 is created, so I need to go and add it to the high byte, and fix it."*

**Step 3 (The Extra Cycle):**

- Normally, the High Byte doesn't require fixing.

- However, because the address crossed into the next page, the CPU must spend **1 extra clock cycle** just to increment and fix that High Byte before reading the data (total: **5 cycles**).

> **Rule:**If `Low Byte + X > 255`, the CPU needs an extra clock cycle just to fix the high byte.
>
> **Rule:**If adding `X` makes the target address cross a page boundary, the carry notification forces an extra cycle to fix the high byte.

**Addressing Mode: Indexed Indirect (`($nn, X)`)**

- **Mental Model:** A pointer table lookup: `*pointer_table[X]`

- **Summary:** The instruction gives a Zero Page base address. Adding `X` points to a 16-bit address (a **Vector**) stored in Zero Page. The CPU reads that Vector to get the final target address (**Effective Address**) in memory.

- #### **Step-by-Step Execution (Example: `LDA ($02, X)` with `X = 4`)**

  1. **Step 1: Calculate the Vector (Zero Page pointer)**

     - Add `X` directly to the instruction's Zero Page address:

       `$02 + 4 = $06`

     - The Vector is at `$0006`.

  2. **Step 2: Read the Effective Address from the Vector**

     - Read 2 consecutive bytes starting at the Vector address (Little-Endian):
       - Low Byte at `$0006`: e.g., `$00`
       - High Byte at `$0007`: e.g., `$80`
     - Combine them into a full 16-bit address: **`$8000`**.

  3. **Step 3: Access the Final Data**

     - The CPU goes to `$8000` and reads the value into the Accumulator (`A`).

Addressing Mode: Indrirect Indexed

**Addressing Mode: Indirect Index (`($nn), Y`)**

**Mental Model:** Pointer offset / array indexing: `ptr[Y]` or `*(ptr + Y)`

**Summary:** The instruction gives a Zero Page address containing a 16-bit base pointer. The CPU reads this pointer and adds the `Y` register to it to calculate the final effective address.

#### **Execution Steps (Example: `LDA ($20), Y` with `Y = $05`)**

1. **Fetch Pointer:** The CPU reads a 16-bit address from Zero Page `$20` and `$21` (e.g., `$4000`).

2. **Add Y:** The CPU adds the 8-bit `Y` register to that 16-bit base address:

   `$4000 + $05 = $4005`

3. **Access Target:** The CPU reads the data from `$4005` into the Accumulator.

4. #### **Page Boundary & Timing Rules**

   - **Base Cycles (Read instructions):** **5 cycles** if within the same page.
   - **Page Crossing Penalty:** If adding `Y` makes the target address cross the page boundary (Low Byte + Y > 255), the CPU needs **1 extra cycle** to fix the high byte (total: **6 cycles**).
   - **Write instructions (e.g., `STA`):** Always take **6 cycles** regardless of page crossing.

**STA(Store Accumulator)(No Flags affected):** 

Load memory from accumulator.

# Jump to Subroutine(JSR)

![JSR opcode table](F:\6502-Emulator-C-20\JSR opcode table.png)

Pushes the return address minus one (PC - 1) onto the stack, then sets the program counter to the target memory address.

*Prerequisites:* A usable stack, a 16-bit address.

**Addressing Mode: Absolute**

Instructions using absolute address mode contain a full 16-bit address to identify the target location.

**Implemantation:**

1. Read a 16-bit address from memory using `FetchWord()`:   
   -  Reads the low byte, then the high byte, updating PC along the way and decrementing execution cycles accordingly.
2. Calculate the return address as the current program counter minus one (`PC - 1`).
3. Set the program counter to the subroutine address.

Cycle Summary (6 Cycles): 

* 1 cycle to fetch the Opcode ($20). 
* 2 cycles to fetch the 16-bit target address via `FetchWord()`. 
* 2 cycles to push the 2-byte return address (`PC - 1`) onto the stack. 
* 1 internal cycle to let the PC jump onto the address I want to jump to.

Expected Effect: Executes a jump to a subroutine (e.g., $7777), pushing the return address onto the stack before modifying PC to point to the target address.

**Note:** As a Little-Endian processor, the first byte read from memory for a 16-bit word is always the Least Significat Byte (LSB).
