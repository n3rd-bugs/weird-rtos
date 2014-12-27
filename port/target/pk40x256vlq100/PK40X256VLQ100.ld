ENTRY(system_entry)

heap_size   = 0;
stack_size  = 0x800;

MEMORY
{
    m_interrupts    (RX): ORIGIN = 0x1FFF0000, LENGTH = 0x000001E0
    m_flash_config  (RX): ORIGIN = 0x00000400, LENGTH = 0x00000010
    m_text          (RX): ORIGIN = 0x20000000, LENGTH = 0x00010000
    m_data          (RW): ORIGIN = 0x1FFF01E0, LENGTH = 0x0000FE20
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
    } > m_text
    
    . = ALIGN(4);
    .bss :
    {
        . = ALIGN(4);
        *(.bss)
        *(.bss*)
        *(COMMON)
    } > m_text
    
    .heap_stack :
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        PROVIDE ( os_stack_end = . );
        . = . + heap_size;
        . = . + stack_size;
        . = ALIGN(4);
    } > m_data
    
    .ARM.attributes 0 : 
    { 
        *(.ARM.attributes) 
    }
}
