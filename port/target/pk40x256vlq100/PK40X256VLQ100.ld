ENTRY(system_entry)

__heap_size   = 0x800;
__stack_size  = 0x800;

MEMORY
{
    m_interrupts    (RX): ORIGIN = 0x00000000, LENGTH = 0x000001E0
    m_flash_config  (RX): ORIGIN = 0x00000400, LENGTH = 0x00000010
    m_text          (RX): ORIGIN = 0x00000800, LENGTH = 0x0003F800
    m_data_l        (RW): ORIGIN = 0x1FFF8000, LENGTH = 0x00008000
    m_data_h        (RW): ORIGIN = 0x20000000, LENGTH = 0x00008000
}

SECTIONS
{
    . = ALIGN(4);
    .interrupts :
    {
        KEEP(*(.system_isr_table))
    } > m_interrupts

    . = ALIGN(4);
    .cfmprotect :
    {
        KEEP(*(.cfmconfig))
    } > m_flash_config

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
    } > m_data_l

    . = ALIGN(4);
    .bss :
    {
        _sbss = .;

        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    } > m_data_l

    .heap_stack :
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + __heap_size;
        . = . + __stack_size;
        PROVIDE ( sys_stack_start = . );
    } > m_data_l

    . = ALIGN(4);
    .static_mem :
    {
        PROVIDE ( static_start = . );
        . = ORIGIN(m_data_l) + LENGTH(m_data_l);
        PROVIDE ( static_end = . );
    } > m_data_l


    . = ALIGN(4);
    .dynamic_mem :
    {
        PROVIDE ( dynamic_start = . );
        . = ORIGIN(m_data_h) + ( LENGTH(m_data_h) - (__heap_size + __stack_size) );
        PROVIDE ( dynamic_end = . );
    } > m_data_h
}
