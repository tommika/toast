MEMORY
{
    ram : ORIGIN = 0x8000, LENGTH = 0x80000
}

SECTIONS
{
    .init : { *(.init*) } > ram
    .text : { *(.text*) } > ram
    .bss(NOLOAD) : { *(.bss*) } > ram
}
