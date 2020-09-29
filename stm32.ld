MEMORY
{
  FLASH (rx): ORIGIN = 0x08000000, LENGTH = 128K
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 8K
}

SECTIONS
{
    .  = 0x0;         /* From 0x00000000 */
    .text : 
    {
        . = ALIGN(4);
        *(vectors)  /* Vector table */
        *(.text)      /* Program code */
        *(.rodata)    /* Read only data */
        *(.glue_7)
        *(.glue_7t)
        . = ALIGN(4);
    } >FLASH

    .  = 0x20000000;  /* From 0x20000000 */ 
    .data : 
    {
        *(.data)      /* Data memory */
    } >FLASH

    .bss :
    {
        *(.bss)       /* Zero-filled run time allocate data memory */
    } >RAM
} 
