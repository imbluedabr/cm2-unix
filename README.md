> [!NOTE]
> Develpoment of this repository has ended but a new project in brewing up will eventually replace this.

# CM2-UNIX

![Kernel Build](https://github.com/imbluedabr/cm2-unix/actions/workflows/c-cpp.yml/badge.svg)

CM2-UNIX was an attempt at creating a unix-ish operating system for an rv32i system built in a roblox game called Circuit Maker 2. But it is now also being ported to run on real hardware like the NXP MCXA153 ARM Microcontrollers.

This is chalanging since the rv32i implementation does not include the privileged architecture or DMA thus we can only ever hope to get cooperative multitasking working, and that's not all, the system only has 64kb of memory and 64kb of external storage. This means that all io operations are PIO and thus extremely slow, but the ports to real hardware will be using interrupts and DMA.

## Dependencies

- GNU Make
- risc64-unkown-elf toolchain or another one(depending on the target board)
- Kconfig frontend, on debian this is called kconfig-frontends
- python 3 (only tested on 3.11)

## Building

1. Clone the repository.
2. Navigate to the root of the repo.
3. run `make menuconfig`
4. run `make all`
5. profit

You can also get a compile_commands.json by doing the following in the project root:
`
bear -- make rebuild
`

## Running

1. run `git submodule init && git submodule update`
2. run `make tools`
3. run `make run`

## Contributing

Contributions are very much apreciated, the project is still in the early stages so a lot will change.


