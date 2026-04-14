#include <stdio.h>
#include <string.h>

#include "lz78.h"

/*
 *
 * How to run:
 *   lz78_main -c <input> <output>   (compress a file)
 *   lz78_main -d <input> <output>   (decompress a file)
 *
 * The parser goes through these steps in order:
 *   START  - read the flag (-c or -d)
 *   INPUT  - read the input file path
 *   OUTPUT - read the output file path
 *   END    - all arguments have been read, ready to run
 *   ERROR  - something was wrong with the arguments
 */

typedef enum {
    START,
    INPUT,
    OUTPUT,
    END,
    ERROR
} ParserState;

typedef enum {
    NONE,
    COMPRESS,
    DECOMPRESS
} Mode;

static void print_usage(const char *prog) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s -c <input> <output>   Compress a file\n", prog);
    fprintf(stderr, "  %s -d <input> <output>   Decompress a file\n", prog);
}

int main(int argc, char *argv[]) {
    ParserState state;
    Mode mode;
    const char *input_path;
    const char *output_path;
    int i;

    state       = START;
    mode        = NONE;
    input_path  = NULL;
    output_path = NULL;

    for (i = 1; i < argc && state != ERROR; i++) {
        switch (state) {

        case START:
            if (strcmp(argv[i], "-c") == 0) {
                mode  = COMPRESS;
                state = INPUT;
            } else if (strcmp(argv[i], "-d") == 0) {
                mode  = DECOMPRESS;
                state = INPUT;
            } else {
                fprintf(stderr, "Error: unknown flag '%s'\n", argv[i]);
                state = ERROR;
            }
            break;

        case INPUT:
            input_path = argv[i];
            state      = OUTPUT;
            break;

        case OUTPUT:
            output_path = argv[i];
            state       = END;
            break;

        case END:
            fprintf(stderr, "Error: unexpected argument '%s'\n", argv[i]);
            state = ERROR;
            break;

        default:
            break;
        }
    }

    /* If we didn't reach END something went wrong */
    if (state == ERROR) {
        print_usage(argv[0]);
        return 1;
    }

    if (state == START) {
        fprintf(stderr, "Error: no mode flag provided (-c or -d)\n");
        print_usage(argv[0]);
        return 1;
    }

    if (state == INPUT) {
        fprintf(stderr, "Error: missing input file path\n");
        print_usage(argv[0]);
        return 1;
    }

    if (state == OUTPUT) {
        fprintf(stderr, "Error: missing output file path\n");
        print_usage(argv[0]);
        return 1;
    }

    /* Make sure the input file actually exists */
    {
        FILE *check = fopen(input_path, "rb");
        if (check == NULL) {
            fprintf(stderr, "Error: input file '%s' does not exist or cannot be read\n", input_path);
            return 1;
        }
        fclose(check);
    }

    /* Run compress or decompress depending on the flag given */
    if (mode == COMPRESS) {
        lz78Compress((char *)input_path, (char *)output_path);
        printf("Compressed '%s' -> '%s'\n", input_path, output_path);
    } else {
        lz78Decompress((char *)input_path, (char *)output_path);
        printf("Decompressed '%s' -> '%s'\n", input_path, output_path);
    }

    return 0;
}
