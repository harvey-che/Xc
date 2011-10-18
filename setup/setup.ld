OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    . = 0;
	.bstext : {*(.bstext)}
	.bsdata : {*(.bsdata)}
	.inittext : {*(.inittext)}
	.initdata : {*(.initdata)}

    .text : {*(.text)}
	.text32 : {*(.text32)}

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
    
	_end = .;

	.eh_frame : {*(.eh_frame)}

	/* /DISCARD/ : {*(.eh_frame)} */
	. = ASSERT(_end <= 0x2200, "Setup too big!");
}
