#include <Xc/kernel.h>

int printk(const char *format, ...)
{
    return 0;
}


int video_area_row = 2;
int video_area_column = 0;


char *itoah(char *str, unsigned long  num)
{
    char *p = str;
	char ch;
	int i;
	int flag = 0;

	*p++ = '0';
	*p++ = 'x';
	
	for (i = 28; i >= 0; i -= 4)
	{
       ch = (num >> i) & 0xf;

	   ch += '0';
	   if (ch > '9')
		   ch += 7;
	   *p++ = ch;
	}
	*p = 0;

	return str;
}

int early_print_str(const char *str)
{   
	asm volatile(
				 "1: movb (%0), %%bl\n\t"
				 "incl %0\n\t"

				 /* test if it's the end of str */
				 "testb %%bl, %%bl\n\t"
				 "jz 4f\n\t"

				 /* handle \n */
				 "cmpb $0x0a, %%bl\n\t"
				 "jnz 5f\n\t"
				 "movl $0, video_area_column\n\t"
				 "incb video_area_row\n\t"
				 "jmp 1b\n\t"

				 /* handle new full row  condition */
				 "5: cmpb $80, video_area_column\n\t"
				 "jnz 2f\n\t"
				 "movl $0, video_area_column\n\t"
				 "incb video_area_row\n\t"
				 "cmpb $25, video_area_row\n\t"
				 "jnz 2f\n\t"

				 "pushl %0\n\t"

				 /* move the whole video area upward by one row */
				 "movl $20 * 24, %%ecx\n\t"
				 "movl $0xb8000 + 160, %0\n\t"
				 "movl $0xb8000, %%edi\n\t"
				 "rep;movsl\n\t"

				 "movl $20, %%ecx\n\t"
				 "xorl %%eax, %%eax\n\t"
				 "movl $0xb8000 + 24 * 80, %%edi\n\t"
				 "rep;stosl\n\t"

                 "popl %0\n\t"
				 "decb video_area_row\n\t"  /* From here, row == 24 */

				 /* calculate the current printing position */
                 "2: xorl %%eax, %%eax\n\t"
                 "xorl %%ecx, %%ecx\n\t"
				 "movb video_area_row, %%cl\n\t"
				 "movb $80, %%al\n\t"
				 "mulb %%cl\n\t"
				 "addw video_area_column, %%ax\n\t"
				 "addw %%ax, %%ax\n\t"
				 "addl $0xb8000, %%eax\n\t"
				 "movl %%eax, %%edi\n\t"

				 "movb %%bl, %%al\n\t"
				 "movb $0x0f, %%ah\n\t"
				 "movw %%ax, (%%edi)\n\t"
				 "incb video_area_column\n\t"
				 "jmp 1b\n\t"
				 "4:"
				 :
				 :"S"(str)
				 :"eax", "ebx", "ecx", "edi", "memory");
	return 0;
}
