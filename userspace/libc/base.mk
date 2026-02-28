

BASE_CFLAGS = -march=rv32i -mabi=ilp32 -mcmodel=medany -fPIC -msmall-data-limit=0 -ffreestanding -nostdlib -nostartfiles
BASE_LDFLAGS = -march=rv32i -mabi=ilp32 -mcmodel=medany -static -Wl,--no-relax -ffreestanding -nostdlib -nostartfiles




