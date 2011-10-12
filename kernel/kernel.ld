OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(startup_32)

SECTIONS
{
    . = 0;
    .text : {*(.text)}

	. = ALIGN(16);
	.rodata : {*(.rodata*)}

	. = ALIGN(16);
	.data : {*(.data*)}

	. = ALIGN(16);
    .bss : {
        __bss_start = .;
		*(.bss)
	    __bss_end = .;
	}

	. = ALIGN(16);
	_end = .;
	/DISCARD/ : {*(.eh_frame)}
	/* . = ASSERT(_end <= 0xA00, "Setup too big!"); */
}
