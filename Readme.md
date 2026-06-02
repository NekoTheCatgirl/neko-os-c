# Neko OS

A small simple microkernel

## Building:

You will need:
- A gcc compiler with x86_64 elf support
- Netwide Assembler (NASM)
- Grub
- Xorriso
- Qemu (optional, for running in using the run target)

### Building the ISO (and all its dependencies):
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target iso
```
This will compile all the submodules, link them, and build the ISO using the grub config

### Running the VM:
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target run
```
Same as building the ISO, it depends on the ISO target. Then it will run it via QEMU.

### Building the ISO on Windows:
```bat
./build.bat
```
Running this requires you to have Docker installed, that's it.

It will first build the minimal iso builder, then it will execute the required build steps using the current dir as its volume.

## License
This project is protected under a reference-only license. Please refer to the [License.md](License.md) file for more information.