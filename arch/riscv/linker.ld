ENTRY(_start)

MEMORY
{
  ROM (rx)   : ORIGIN = 0x0000, LENGTH = 16K
  RAM (rwx)  : ORIGIN = 0x8000, LENGTH = 16K
  HEAP (rwx) : ORIGIN = 0x8800, LENGTH = 16K
  mmio (rwx) : ORIGIN = 0xFFC1, LENGTH = 0xFFFF - .
}

SECTIONS
{
  .text : {
    *(.text*)
  } > ROM

  .rodata : {
    *(.rodata*)
    *(.srodata*)
  } > ROM

  .data : {
     *(.data*) 
  } > RAM

  .bss  : {
    *(.bss*)
    *(COMMON) 
  } > RAM

  _heap_start = .;
  _stack_top = 0xFFC0;
  _heap_end = _stack_top - 1;
}
