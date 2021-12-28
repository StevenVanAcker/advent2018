#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#define BUFSIZE (1024*1024)
#define CHARSET \
	"__     _  __ \n" \
	"| \\__ `\\O/  `--  {}    \\}    {/\n" \
	"\\    \\_(~)/______/=____/=____/=*\n" \
	" \\=======/    //\\\\  >\\/> || \\> \n" \
	"----:---:---  `` `` ```` `` ``\n" \
	"\n" \
	"   *        *        *        __@    *       *\n" \
	"*      *       *        *    /_| ^     *\n" \
	"   K  *     K      *        o'_)/ \\  *    *\n" \
	"  <')____  <')____    __*   V   \\  ) __  *\n" \
	"   \\ ___ )--\\ ___ )--( (    (___|__)/ /*     *\n" \
	" *  |   |    |   |  * \\ \\____] [___/ /  *\n" \
	"    |*  |    |   |     \\____________/       *\n"


#define ENDCHAR '\xff'

char *addr;

void main() {
	void (*func)();
	size_t readsize = 0;

	addr = mmap(NULL, BUFSIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED) {
		printf("Could not map memory\n");
		return;
	}

	/*
	printf("code at %p\n", addr);
	fflush(0);
	*/

	for(size_t i = 0; i < BUFSIZE; i++) {
		char c;

		if(!fread(&c, 1, 1, stdin)) {
			printf("EOF :(\n");
			return;
		}

		if(c == ENDCHAR) {
			// done
			break;
		}

		if(!strchr(CHARSET, c) && c != '\x00') {
			printf("Invalid char %02x at offset %d :(\n", c, i);
			return;
		}

		addr[i] = c;
	}

	printf("Looks like valid ASCII art to me!!\n");
	fflush(0);
	func = (void (*))addr;
	func();
}
