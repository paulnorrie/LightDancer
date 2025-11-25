# LightDancer
Make LED strips dance to music on a Raspberry Pi Pico (RP2040).


# Hardware

# Building Software

## Software Pre-requisites

`cmake -B build --DPICO_BOARD=pico_w` (change board as needed)
`cmake --build build`

## Command-line

## With VS Code
Add Raspberry Pi Pico extension
Import Project
The VS Code extension uses Ninja as a build tool instead of Make.  If you have built this on the
command-line prior then it may fail to compile in VS Code.  The developer tools (Help > Toggle Developer Tools)
console will show an error like 
```
CMake Error: Error: generator : Ninja
Does not match the generator used previously: Unix Makefiles
Either remove the CMakeCache.txt file and CMakeFiles directory or choose a different binary directory
```
then select 'Configure CMake' in the Command Palette in VS Code with the


## Software Notes
# No Heap
Dynamic memory allocation on the heap leads to memory fragmentation and with only 264Kb of RAM in total
on the RP2040, there is not much for the heap so any fragmentation may take up needed space.  Additionally
there is some overhead in managing this.

The default memory model (as of SDK version 2.2) is defined in the SDK in linker scripts for the
[RP2040](https://github.com/raspberrypi/pico-sdk/blob/master/src/rp2_common/pico_crt0/rp2040/memmap_default.ld) and
the [RP2350](https://github.com/raspberrypi/pico-sdk/blob/master/src/rp2_common/pico_crt0/rp2350/memmap_default.ld)

For the RP2040, it looks like:
  ---------------------------- 0x20042000
  | SCRATCH_Y | Core0 stack  |    4K
  |           | Grows ↓      |
  ---------------------------- 0x20041000
  | SCRATCH_X | Core1 stack  |    4K
  |           | Grows ↓      |
  ---------------------------- 0x20040000
  |           | .heap        |
  |           |              |  size is 
  |           |              |  
  |           |              |
  |           | Grows ↑      |
  |           |--------------| 
  |    RAM    | .bss         |
  |           | (uninit'd    | size depends
  |           | global &     | on program
  |           | static vars) |
  |           |--------------|
  |           | .data        | size depends
  |           | (initialised | on program
  |           | global &     |
  |           | static vars) |
  |           |--------------|
  |           |Boot 2 loader |  256 bytes
  ---------------------------- 0x20000000  

If only using 1 core (Core0), the stack can be bigger than 4K without corrupting Core1 stack since
Core1 is not used.

The Heap can grow all the way to the top of RAM, corrupting the Core1 and Core0 stacks.  Setting
the compiler definition PICO_USE_STACK_GUARDS=1 write-protects (using the ARM Memory Management Unit)
the last word on the stack, causing a hard-fault if that address is written to.

If multiple cores are used, Core0 stack can grow to corrupt Core1 stack and at some point the next
allocations on Core1 stack.  Core1 stack can start at a lower address by calling
[`multicore_launch_core1_with_stack()`](https://github.com/raspberrypi/pico-sdk/blob/a1438dff1d38bd9c65dbd693f0e5db4b9ae91779/src/rp2_common/pico_multicore/multicore.c#L141C6-L141C39)

In summary, the options to avoid using heap allocation are:

1. Use local variables on the stack. Easy but need to track how large Core0 stack has to be so it
Core1 can be launched with a stack pointer at a lower address (linker map files and -fstack-usage helps plan this).  
Or use a custom linker script to put Core1 stack earlier but that's way more involved.

1. Use uninitialised global arrays or pools in the .bss segment.  However the allocation lasts
the lifetime of the program, even if only useful at some times. Needs thread synchronisation if
threads are to be used. Needs a unique namespace so as not to clash with external libraries.
stack variables.
needed, and a little less easy, but less chance of collision with Core1 stack and less chance of
accidentally allocating more memory than available since this can be put in one file instead of
stack variables all over the place.

## Hardware Notes

- The data line is 5V. Operating it from the GPIO directly at 3.3V will cause random colours to 
  illuminate. This is why a logic level shift is required.
- 