Simple, slightly buggy CHIP-8 emulator/interpreter.

To build, have SDL2 and CMake installed. Then follow these steps:

git clone https://github.com/vkoskiv/CHIP8
cd CHIP8
cmake .
make

Then to run:

./bin/CHIP-8 c8games/<GAME NAME>

Hit ^C to stop it.

There are some useful debug options at the start of CPU.h
You can enable a longer delay and a full printout of instructions being run.
This makes debugging your own CHIP-8 programs much easier.
There is also an autohalt feature, which stops the CPU if an infinite loop is detected.

The controls are mapped as follows:

CHIP8 HEX MAP:
+-+-+-+-+
|1|2|3|C|
+-+-+-+-+
|4|5|6|D|
+-+-+-+-+
|7|8|9|E|
+-+-+-+-+
|A|0|B|F|
+-+-+-+-+
HOST KEYBOARD:
+-+-+-+-+
|1|2|3|4|
+-+-+-+-+
|Q|W|E|R|
+-+-+-+-+
|A|S|D|F|
+-+-+-+-+
|Z|X|C|V|
+-+-+-+-+

Feel free to send PRs for feature improvements.
