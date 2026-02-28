

ENTRY(_start)

SECTIONS {
    . = 0;

    .text : {
        KEEP(*(.text.start))
        *(.text*)
    }
    
    .rodata : {
        *(.rodata*)
        *(.srodata*)
    }

    .data : {
        *(.data*)
        *(.sdata*)
    }

    .bss : {
        *(.bss*)
        *(.sbss)
        *(COMMON)
    }

    . = ALIGN(16);
    _stack_top = . + 2K;
    
}





