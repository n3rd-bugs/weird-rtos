ENTRY(system_entry)

__heap_size   = 0x800;
__stack_size  = 0x800;

MEMORY
{
    m_interrupts    (RX): ORIGIN = 0x08000000, LENGTH = 0x000001E0
    m_text          (RX): ORIGIN = 0x080001E0, LENGTH = 0x000FFE20
    m_data          (RW): ORIGIN = 0x20000000, LENGTH = 0x00020000
    m_cram          (RX): ORIGIN = 0x10000000, LENGTH = 0x00010000
}

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > m_interrupts

    .text :
    {
        . = ALIGN(4);
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        *(.glue_7)         /* glue arm to thumb code */
        *(.glue_7t)        /* glue thumb to arm code */
        *(.eh_frame)
        
        KEEP (*(.init))
        KEEP (*(.fini))
        
        . = ALIGN(4);
        _etext = .;        /* define a global symbols at end of code */
    } > m_text


    .ARM.extab : 
    { 
        *(.ARM.extab* .gnu.linkonce.armextab.*) 
    } > m_text

    .ARM : 
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > m_text

    .data :
    {
        . = ALIGN(4);
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
    } > m_data

    . = ALIGN(4);
    .bss :
    {
        *(.bss)
        *(.bss*)
        *(COMMON)
    } > m_data
    
    .dynamic_mem :
    {
        PROVIDE ( dynamic_start = . );
        . = ORIGIN(m_data) + ( LENGTH(m_data) - (__heap_size + __stack_size) );
        PROVIDE ( dynamic_end = . );
    } > m_data

    /* User_heap_stack section, used to check that there is enough RAM left */
    ._user_heap_stack :
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + __heap_size;
        . = . + __stack_size;
        PROVIDE ( os_stack_end = . );
        . = ALIGN(4);
    } > m_data

    .static_mem :
    {
        PROVIDE ( static_start = . );
        . = ORIGIN(m_cram) + LENGTH(m_cram);
        PROVIDE ( static_end = . );
    } > m_cram

    .ARM.attributes 0 : 
    { 
        *(.ARM.attributes) 
    }
}