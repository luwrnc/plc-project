#include <stdio.h>
#include <string.h>

#include "lz78.h"

/* Small CLI wrapper:
 *   -c <input> <output>
 *   -d <input> <output>
 */

static void print_usage(const char *prog) {
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  %s -c <input> <output>\n", prog);
	fprintf(stderr, "  %s -d <input> <output>\n", prog);
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		print_usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "-c") == 0) {
		lz78Compress(argv[2], argv[3]);
		printf("Compressed '%s' -> '%s'\n", argv[2], argv[3]);
		return 0;
	}

	if (strcmp(argv[1], "-d") == 0) {
		lz78Decompress(argv[2], argv[3]);
		printf("Decompressed '%s' -> '%s'\n", argv[2], argv[3]);
		return 0;
	}

	print_usage(argv[0]);
	return 1;
}