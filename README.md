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



## Notes

- The data line is 5V. Operating it from the GPIO directly at 3.3V will cause random colours to 
  illuminate. This is why a logic level shift is required.
- 