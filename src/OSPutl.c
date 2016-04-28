#include"../include/OSPlib.h"

void OSPrint(uint64_t level, const char *format, ...) {
	static uint64_t lvl = 0;
	char buf[BUFSIZ];

	if(!format) {
		lvl = level;
		return;
	}

	if(level < lvl) return;

	va_list arg;
	va_start(arg, format);
	vsprintf(buf, format, arg);
	va_end(arg);

	printf("OSPLog %3lu : %s\n", level, buf);
}

void *OSPArray(size_t cell, ...) {
    unsigned int dim[256];
    va_list arg;
	uint8_t *ret;
	uint8_t *dst;
	void **src;
	size_t it = 0;
	size_t size = 0;
	unsigned int acc = 1;
	
	va_start(arg, cell);
	while((dim[it++] = va_arg(arg, unsigned int)));
	va_end(arg);

	if(dim[0] && cell) {
		for(it = 0; dim[it]; it++) {
			acc *= dim[it];

			if(dim[it + 1]) {
				size += acc;
			}
			else {
				size = size * sizeof(void *) + acc * cell;
			}
		}

		acc = 1;
		ret = malloc(size);
		src = (void **) ret;
		dst = ret + (dim[0] * sizeof(void *));

		for(it = 0; dim[it + 1]; it++) {
			acc *= dim[it];

			for(size = 0; size < acc; size++) {
				*(src++) = (void *) dst;

				if(dim[it + 2]) { 
					dst += (dim[it + 1] * sizeof(void *));
				}
				else {
					dst += (dim[it + 1] * cell);
				}
			}
		}
	}
	else {
		ret = 0;
	}

	return (void *)ret;
}

