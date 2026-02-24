

BASE_CFLAGS = -march=rv32i -mabi=ilp32 -ffreestanding -fPIC -msmall-data-limit=0 -nostdlib -nostartfiles
BASE_LDFLAGS = -march=rv32i -mabi=ilp32 -ffreestanding -Wl,--no-relax -nostdlib -nostartfiles




