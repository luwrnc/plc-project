#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "lz78.h"

/* Test script

   Edit TEST_FILES below to list the files you want to test.
   Each file is compressed and decompressed, then checked for round-trip equality.
*/

#define ARTIFACT_DIR "tests/artifacts"
#define MAX_PATH_LEN 1024

static const char *TEST_FILES[] = {
    "tests/images/1675398821215.jpeg",
    "tests/images/images_2.jpeg",
    "tests/texts/mobydick.txt",
    "tests/texts/sherlockholmes.txt"
};

#define TEST_FILE_COUNT (sizeof(TEST_FILES) / sizeof(TEST_FILES[0]))

static const char *base_name(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash == NULL ? path : slash + 1;
}

static int ensure_artifact_dir(void) {
    if (mkdir(ARTIFACT_DIR, 0755) == 0 || errno == EEXIST) {
        return 1;
    }
    return 0;
}

static int read_file(const char *path, unsigned char **data, size_t *size) {
    FILE *fp;
    long file_len;
    unsigned char *buf;
    size_t n;

    *data = NULL;
    *size = 0U;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open %s\n", path);
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }

    file_len = ftell(fp);
    if (file_len < 0) {
        fclose(fp);
        return 0;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    if (file_len == 0) {
        fclose(fp);
        return 1;
    }

    buf = (unsigned char *)malloc((size_t)file_len);
    if (buf == NULL) {
        fclose(fp);
        return 0;
    }

    n = fread(buf, 1U, (size_t)file_len, fp);
    fclose(fp);

    if (n != (size_t)file_len) {
        free(buf);
        return 0;
    }

    *data = buf;
    *size = (size_t)file_len;
    return 1;
}

static int files_identical(const char *a, const char *b) {
    unsigned char *data_a;
    unsigned char *data_b;
    size_t size_a;
    size_t size_b;
    int ok;

    data_a = NULL;
    data_b = NULL;
    size_a = 0U;
    size_b = 0U;

    if (!read_file(a, &data_a, &size_a)) {
        return 0;
    }

    if (!read_file(b, &data_b, &size_b)) {
        free(data_a);
        return 0;
    }

    ok = (size_a == size_b) && (size_a == 0U || memcmp(data_a, data_b, size_a) == 0);

    free(data_a);
    free(data_b);
    return ok;
}

int main(void) {
    unsigned char *original;
    unsigned char *compressed;
    size_t original_size;
    size_t compressed_size;
    size_t i;
    int had_failure;

    had_failure = 0;

    if (!ensure_artifact_dir()) {
        fprintf(stderr, "Cannot create %s\n", ARTIFACT_DIR);
        return 1;
    }

    for (i = 0U; i < TEST_FILE_COUNT; i++) {
        const char *input_path = TEST_FILES[i];
        const char *name = base_name(input_path);
        char compressed_path[MAX_PATH_LEN];
        char decompressed_path[MAX_PATH_LEN];

        original = NULL;
        compressed = NULL;
        original_size = 0U;
        compressed_size = 0U;

        if (snprintf(compressed_path, sizeof(compressed_path), "%s/%s.lz78", ARTIFACT_DIR, name) >= (int)sizeof(compressed_path) ||
            snprintf(decompressed_path, sizeof(decompressed_path), "%s/%s.roundtrip.c", ARTIFACT_DIR, name) >= (int)sizeof(decompressed_path)) {
            fprintf(stderr, "Path too long for input %s\n", input_path);
            had_failure = 1;
            continue;
        }

        printf("\nCompressing %s\n", input_path);
        lz78Compress((char *)input_path, compressed_path);

        printf("Decompressing %s\n", compressed_path);
        lz78Decompress(compressed_path, decompressed_path);

        if (!files_identical(input_path, decompressed_path)) {
            fprintf(stderr, "Round-trip FAILED for %s\n", input_path);
            had_failure = 1;
            continue;
        }

        if (read_file(input_path, &original, &original_size) &&
            read_file(compressed_path, &compressed, &compressed_size) &&
            original_size > 0U) {
            double ratio = (double)compressed_size / (double)original_size;
            printf("Original:   %zu bytes\n", original_size);
            printf("Compressed: %zu bytes\n", compressed_size);
            printf("Ratio:      %.3f\n", ratio);
        }

        free(original);
        free(compressed);
        printf("Round-trip PASSED for %s\n", input_path);
    }

    if (had_failure) {
        fprintf(stderr, "\nOne or more files failed round-trip checks\n");
        return 1;
    }

    printf("\nAll files passed round-trip checks\n");
    return 0;
}