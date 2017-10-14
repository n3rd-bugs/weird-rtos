ENTRY(system_entry)

MEMORY
{
    m_interrupts    (RX): ORIGIN = 0x08000000, LENGTH = 0x000001E0
    m_text          (RX): ORIGIN = 0x080001E0, LENGTH = 0x000FFE20
    m_data          (RW): ORIGIN = 0x20000000, LENGTH = 0x00020000
    m_cram          (RX): ORIGIN = 0x10000000, LENGTH = 0x00010000
}

heap_size = 512;

SECTIONS
{
    . = ALIGN(4);
    .interrupts :
    {
         KEEP(*(.system_isr_table))
    } > m_interrupts

    . = ALIGN(4);
    .text :
    {
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
    } > m_text

    . = ALIGN(4);
    _sidata = .;

    .data : AT ( _sidata )
    {
        _sdata = .;
        *(.data)
        *(.data*)

        . = ALIGN(4);
        _edata = .;
    } > m_data

    . = ALIGN(4);
    .bss :
    {
        _sbss = .;

        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;

    } > m_data

    . = ALIGN(4);
    .heap :
    {
        PROVIDE(end = .);
        PROVIDE(_heap_start = .);

        . = . + heap_size;

        PROVIDE(_heap_end = .);
        PROVIDE(_stack_start = .);
    } > m_data
}