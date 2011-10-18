OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(startup_32)

SECTIONS
{
    . = 0xc0100000;
    .text : {
		_text = .;
		*(.text)
		_etext = .;
	}

	. = ALIGN(16);
	.rodata : {
		_srodata = .;
		*(.rodata*)
		_erodtata = .;
	}

	. = ALIGN(16);
	.data : {
		_sdata = .;
		*(.data*)
		_edata = .;
	}

	. = ALIGN(4096);
    .bss : {
        __bss_start = .;
		*(.bss)
		. = ALIGN(4096);
	    __bss_stop = .;
	}

    . = ALIGN(4096);
	.brk : {
        __brk_base = .;
        . += 64 * 1024;
		__brk_limit = .;
	}

	_end = .;

	.eh_frame : {*(.eh_frame)}

	/* /DISCARD/ : {*(.eh_frame)} */
	. = ASSERT( . <= 0xc0180000, "Kernel too big!");
}
