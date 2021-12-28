#include "config.h"
#include "tools.h"

void nop() { }

void *myalloc(size_t size) {
	void *x = calloc(size, 1);
	if(!x) {
		ERROR("Couldn't allocate memory\n");
	}
	//DEBUG("%lu bytes allocated at %p\n", size, x);
	return x;
}

void _myfree(void *p) {
	if(!p) return;
	//DEBUG("memory at %p freed\n", p);
	free(p);
}

void LOGFN(const char *type, const char *format, ...) {
    va_list arg;
	if(!globDebugFlag && !strcmp(type, "DEBUG")) return;
    va_start(arg, format);
    printf("[%s] ", type);
    vfprintf(stdout, format, arg);
    va_end(arg);
}  

void sha256sum(uint8_t *data, size_t datalen, char *outputhash) {
    unsigned char rawhash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, datalen);
    SHA256_Final(rawhash, &sha256);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputhash + (i * 2), "%02x", rawhash[i]);
    }
    outputhash[64] = 0;
}

int OCD_satisfied(uint8_t *data, size_t datalen) {
	char buf[65];
	sha256sum(data, datalen, buf);
	return !strncmp(buf, OCD_prefix, strlen(OCD_prefix));
}

void random_string(uint8_t *data, size_t datalen) {
	uint8_t *charpool = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	size_t charpoollen = strlen(charpool);
	if(!datalen) return;

	for(int i = 0; i < datalen - 1; i++) {
		size_t rnd = random() % charpoollen;
		data[i] = charpool[rnd];
	}

	data[datalen - 1] = '\0';
}

// myalloc called, must free (only called from read_file_as_string)
void read_file(char *filename, uint8_t **data, size_t *len) {
	FILE *in = fopen(filename, "rb");

	if(!in) { 
		*data = NULL; 
		*len = 0; 
		return; 
	}

	fseek(in, 0, SEEK_END);
	*len = ftell(in);
	fseek(in, 0, SEEK_SET);

	*data = myalloc(*len);
	fread(*data, *len, 1, in);
	fclose(in);
}

void write_file(char *filename, uint8_t *data, size_t len) {
	FILE *out = fopen(filename, "wb");
	fwrite(data, len, 1, out);
	fclose(out);
}

// realloc called, must free (only called from read_file_as_string)
uint8_t *buf2str(uint8_t *data, size_t size) {
	if(!data) return NULL;

	data = (uint8_t *)realloc(data, size + 1);
	data[size] = '\0';
	return data;
}

// myalloc called, must free
uint8_t *read_file_as_string(char *filename) {
	uint8_t *data;
	size_t len;
	read_file(filename, &data, &len);
	return strip(buf2str(data, len));
}

char *dotfile(char *filename) {
	static char output[1000];
	snprintf(output, sizeof(output), "%s/%s", getenv("HOME"), filename);
	return output;
}

void hexdump(void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

char *strip(char *msg) {
	while(msg && strlen(msg) > 0 && msg[strlen(msg) - 1] == '\n') {
		msg[strlen(msg) - 1] = '\0';
	}
	return msg;
}
