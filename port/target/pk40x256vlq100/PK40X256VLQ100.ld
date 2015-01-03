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
    .interrupts :
    {
        __vector_table = .;
        . = ALIGN(4);
        KEEP(*(.vectortable))
        . = ALIGN(4);
    } > m_interrupts

    .cfmprotect :
    {
        . = ALIGN(4);
        KEEP(*(.cfmconfig))
        . = ALIGN(4);
    } > m_flash_config

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)

        . = ALIGN(4);

        ___ROM_AT = .;
    } > m_text

    .data :
    {
        . = ALIGN(4);
        *(.data)
        *(.data*)
    } > m_data_l

    . = ALIGN(4);
    .bss :
    {
        . = ALIGN(4);
        *(.bss)
        *(.bss*)
        *(COMMON)
    } > m_data_l

    .heap_stack :
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        __heap_addr = .;
        . = . + __heap_size;
        . = . + __stack_size;
        . = ALIGN(4);
        PROVIDE ( os_stack_end = . );
    } > m_data_l
    
    .static_mem :
    {
        PROVIDE ( static_start = . );
        . = ORIGIN(m_data_l) + LENGTH(m_data_l);
        PROVIDE ( static_end = . );
    } > m_data_l
    
    .dynamic_mem :
    {
        PROVIDE ( dynamic_start = . );
        . = ORIGIN(m_data_h) + ( LENGTH(m_data_h) - (__heap_size + __stack_size) );
        PROVIDE ( dynamic_end = . );
    } > m_data_h
}
