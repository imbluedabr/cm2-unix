
# CM2-UNIX

![Kernel Build](https://github.com/imbluedabr/cm2-unix/actions/workflows/c-cpp.yml/badge.svg)

CM2-UNIX is an attempt at creating a unix-ish operating system for a rv32i system built in a roblox game called Circuit Maker 2.

This is exceptionally hard since the rv32i implementation does not include traps, thus we can only ever hope to get cooperative multitasking working, and that's not all, the system only has 64kb of memory and 64kb of external storage.

## Building

1. Clone the repository.
2. Navigate to the root of the repo.
3. run `make image`
4. profit

You can also get a compile_commands.json by doing the following in the project root:
`
bear -- make
`

## Contributing

work in progress...

