## Gameboy Emulator (C++)

When the project is near completion, I will provide build instructions and other much needed detail; however, this project is very much early in development! In the meantime, here is a quick list of requirements to attempt to build the project:
- Machine supporting OpenGL 4.6+
- C++23 compliant compiler for Windows/Linux (MSVC 19.44 / GCC 15 / Clang 20). (Linux functionality may break between commits as I'm focusing on Windows first.)
- CMake 3.28+*
- Python*
  - Jinja2*

(* glad is currently downloaded and built automatically in the cmake build script. doing this requires python and the tool jinja to be present on your machine. support for building without this will be added in the future)

If you're curious on what resources I'm using to do this, here's a non-exhaustive list:
- [Gameboy Complete Technical Reference](https://gekkio.fi/files/gb-docs/gbctr.pdf)
- [gbdev Pan Docs](https://gbdev.io/pandocs/)
- [Info on some internals](https://gist.github.com/SonoSooS/c0055300670d678b5ae8433e20bea595#opcode-holes-not-implemented-opcodes)
- [CPU Op Code Mappings](https://gbdev.io/gb-opcodes/optables/)
- [CPU Instruction Explanations](https://rgbds.gbdev.io/docs/v0.9.1/gbz80.7#LD_r8,r8https://rgbds.gbdev.io/docs/v0.9.1/gbz80.7)
- [GameBoy Emulation in JavaScript](https://imrannazar.com/series/gameboy-emulation-in-javascript/input)
- [Blargg's test ROMs](https://gbdev.gg8.se/files/roms/blargg-gb-tests/)
