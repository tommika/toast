MEMORY
{
    ram : ORIGIN = 0x8000, LENGTH = 0x10000
}

SECTIONS
{
    .init : { *(.init*) } > ram
    .text : { *(.text*) } > ram
    .bss : { *(.bss*) } > ram
}
