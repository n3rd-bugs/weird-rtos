ENTRY(system_entry)

MEMORY
{
    m_text          (RX): ORIGIN = 0x8000000,   LENGTH = 0x10000
    m_data          (RW): ORIGIN = 0x20000000,  LENGTH = 0x5000
}

SECTIONS
{
    . = ALIGN(4);
    .text :
    {
         KEEP(*(.interrupts))
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
}