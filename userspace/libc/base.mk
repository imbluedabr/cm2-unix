

BASE_CFLAGS = -march=rv32i -mabi=ilp32 -fPIE -msmall-data-limit=0 -ffreestanding -nostdlib -nostartfiles
BASE_LDFLAGS = -march=rv32i -mabi=ilp32 --static-pie -Wl,--no-relax -ffreestanding -nostdlib -nostartfiles




