#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_DICT_SIZE (1U << 20)
#define HASH_CAPACITY (1U << 21)

static const unsigned char MAGIC_HEADER[5] = {'L', 'Z', '7', '8', 0x02};

typedef struct {
	uint32_t key;
	uint32_t value;
	unsigned char used;
} DictSlot;

static size_t hash_key(uint32_t key) {
	return (size_t)((key * 2654435761U) & (HASH_CAPACITY - 1U));
}

static uint32_t dict_find(const DictSlot *table, uint32_t parent_index, unsigned char next_byte) {
	uint32_t key = (parent_index << 8) | (uint32_t)next_byte;
	size_t i = hash_key(key);

	while (table[i].used) {
		if (table[i].key == key) {
			return table[i].value;
		}
		i = (i + 1U) & (HASH_CAPACITY - 1U);
	}

	return 0U;
}

static int dict_insert(DictSlot *table, uint32_t parent_index, unsigned char next_byte, uint32_t value) {
	uint32_t key = (parent_index << 8) | (uint32_t)next_byte;
	size_t i = hash_key(key);

	while (table[i].used) {
		if (table[i].key == key) {
			table[i].value = value;
			return 1;
		}
		i = (i + 1U) & (HASH_CAPACITY - 1U);
	}

	table[i].used = 1U;
	table[i].key = key;
	table[i].value = value;
	return 1;
}

static int write_u8(FILE *fp, unsigned char value) {
	return fputc((int)value, fp) != EOF;
}

static int write_u32_le(FILE *fp, uint32_t value) {
	unsigned char b[4];
	b[0] = (unsigned char)(value & 0xFFU);
	b[1] = (unsigned char)((value >> 8) & 0xFFU);
	b[2] = (unsigned char)((value >> 16) & 0xFFU);
	b[3] = (unsigned char)((value >> 24) & 0xFFU);
	return fwrite(b, 1U, 4U, fp) == 4U;
}

static int read_u8(FILE *fp, unsigned char *value) {
	int ch = fgetc(fp);
	if (ch == EOF) {
		return 0;
	}
	*value = (unsigned char)ch;
	return 1;
}

static int read_u32_le(FILE *fp, uint32_t *value) {
	unsigned char b[4];
	if (fread(b, 1U, 4U, fp) != 4U) {
		return 0;
	}
	*value = (uint32_t)b[0] |
		((uint32_t)b[1] << 8) |
		((uint32_t)b[2] << 16) |
		((uint32_t)b[3] << 24);
	return 1;
}

static int write_phrase(FILE *dest,
		const uint32_t *parent,
		const unsigned char *symbol,
		uint32_t dict_size,
		uint32_t idx,
		unsigned char *scratch) {
	size_t len = 0U;
	uint32_t cur = idx;

	while (cur != 0U) {
		if (cur >= dict_size || len >= MAX_DICT_SIZE) {
			return 0;
		}
		scratch[len++] = symbol[cur];
		cur = parent[cur];
	}

	while (len > 0U) {
		len--;
		if (fputc((int)scratch[len], dest) == EOF) {
			return 0;
		}
	}

	return 1;
}


void lz78Compress( char * srcFilePath, char * destFilePath){
	FILE *src;
	FILE *dest;
	DictSlot *dictionary;
	uint32_t dict_size;
	uint32_t full_entries;
	uint32_t w_index;
	int ch;
	unsigned char has_tail;

	src = fopen(srcFilePath, "rb");
	if (src == NULL) {
		fprintf(stderr, "Cannot open '%s': %s\n", srcFilePath, strerror(errno));
		return;
	}

	dest = fopen(destFilePath, "wb");
	if (dest == NULL) {
		fprintf(stderr, "Cannot open '%s': %s\n", destFilePath, strerror(errno));
		fclose(src);
		return;
	}

	dictionary = (DictSlot *)calloc(HASH_CAPACITY, sizeof(DictSlot));
	if (dictionary == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		fclose(src);
		fclose(dest);
		return;
	}

	if (fwrite(MAGIC_HEADER, 1U, sizeof(MAGIC_HEADER), dest) != sizeof(MAGIC_HEADER) ||
		!write_u32_le(dest, 0U) ||
		!write_u8(dest, 0U)) {
		fprintf(stderr, "Write error for '%s'\n", destFilePath);
		free(dictionary);
		fclose(src);
		fclose(dest);
		return;
	}

	dict_size = 257U;
	for (ch = 0; ch < 256; ch++) {
		dict_insert(dictionary, 0U, (unsigned char)ch, (uint32_t)ch + 1U);
	}

	full_entries = 0U;
	w_index = 0U;

	for (;;) {
		uint32_t wc_index;
		ch = fgetc(src);
		if (ch == EOF) {
			break;
		}

		wc_index = dict_find(dictionary, w_index, (unsigned char)ch);
		if (wc_index != 0U) {
			w_index = wc_index;
		} else {
			if (dict_size < MAX_DICT_SIZE) {
				dict_insert(dictionary, w_index, (unsigned char)ch, dict_size);
				dict_size++;
			}

			if (!write_u32_le(dest, w_index) || !write_u8(dest, (unsigned char)ch)) {
				fprintf(stderr, "Write error for '%s'\n", destFilePath);
				free(dictionary);
				fclose(src);
				fclose(dest);
				return;
			}

			full_entries++;
			w_index = 0U;
		}
	}

	if (ferror(src)) {
		fprintf(stderr, "Read error for '%s'\n", srcFilePath);
		free(dictionary);
		fclose(src);
		fclose(dest);
		return;
	}

	has_tail = (unsigned char)(w_index != 0U ? 1U : 0U);
	if (has_tail && !write_u32_le(dest, w_index)) {
		fprintf(stderr, "Write error for '%s'\n", destFilePath);
		free(dictionary);
		fclose(src);
		fclose(dest);
		return;
	}

	if (fseek(dest, (long)sizeof(MAGIC_HEADER), SEEK_SET) != 0 ||
		!write_u32_le(dest, full_entries) ||
		!write_u8(dest, has_tail)) {
		fprintf(stderr, "Header update failed for '%s'\n", destFilePath);
		free(dictionary);
		fclose(src);
		fclose(dest);
		return;
	}

	free(dictionary);
	fclose(src);
	fclose(dest);
}
void lz78Decompress( char * srcFilePath, char * destFilePath){
	FILE *src;
	FILE *dest;
	unsigned char header[sizeof(MAGIC_HEADER)];
	uint32_t num_entries;
	unsigned char has_tail;
	uint32_t *parent;
	unsigned char *symbol;
	unsigned char *scratch;
	uint32_t dict_size;
	uint32_t i;

	src = fopen(srcFilePath, "rb");
	if (src == NULL) {
		fprintf(stderr, "Cannot open '%s': %s\n", srcFilePath, strerror(errno));
		return;
	}

	dest = fopen(destFilePath, "wb");
	if (dest == NULL) {
		fprintf(stderr, "Cannot open '%s': %s\n", destFilePath, strerror(errno));
		fclose(src);
		return;
	}

	if (fread(header, 1U, sizeof(header), src) != sizeof(header) ||
		memcmp(header, MAGIC_HEADER, sizeof(MAGIC_HEADER)) != 0) {
		fprintf(stderr, "Invalid LZ78 header in '%s'\n", srcFilePath);
		fclose(src);
		fclose(dest);
		return;
	}

	if (!read_u32_le(src, &num_entries) || !read_u8(src, &has_tail) || (has_tail != 0U && has_tail != 1U)) {
		fprintf(stderr, "Corrupted LZ78 stream in '%s'\n", srcFilePath);
		fclose(src);
		fclose(dest);
		return;
	}

	parent = (uint32_t *)malloc((size_t)MAX_DICT_SIZE * sizeof(uint32_t));
	symbol = (unsigned char *)malloc((size_t)MAX_DICT_SIZE * sizeof(unsigned char));
	scratch = (unsigned char *)malloc((size_t)MAX_DICT_SIZE * sizeof(unsigned char));
	if (parent == NULL || symbol == NULL || scratch == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		free(parent);
		free(symbol);
		free(scratch);
		fclose(src);
		fclose(dest);
		return;
	}

	parent[0] = 0U;
	symbol[0] = 0U;
	dict_size = 257U;
	for (i = 0U; i < 256U; i++) {
		parent[i + 1U] = 0U;
		symbol[i + 1U] = (unsigned char)i;
	}

	for (i = 0U; i < num_entries; i++) {
		uint32_t idx;
		unsigned char next_byte;

		if (!read_u32_le(src, &idx) || !read_u8(src, &next_byte) || idx >= dict_size) {
			fprintf(stderr, "Corrupted LZ78 stream in '%s'\n", srcFilePath);
			free(parent);
			free(symbol);
			free(scratch);
			fclose(src);
			fclose(dest);
			return;
		}

		if (!write_phrase(dest, parent, symbol, dict_size, idx, scratch) ||
			fputc((int)next_byte, dest) == EOF) {
			fprintf(stderr, "Write error for '%s'\n", destFilePath);
			free(parent);
			free(symbol);
			free(scratch);
			fclose(src);
			fclose(dest);
			return;
		}

		if (dict_size < MAX_DICT_SIZE) {
			parent[dict_size] = idx;
			symbol[dict_size] = next_byte;
			dict_size++;
		}
	}

	if (has_tail != 0U) {
		uint32_t idx;
		if (!read_u32_le(src, &idx) || idx >= dict_size) {
			fprintf(stderr, "Corrupted LZ78 stream in '%s'\n", srcFilePath);
			free(parent);
			free(symbol);
			free(scratch);
			fclose(src);
			fclose(dest);
			return;
		}

		if (!write_phrase(dest, parent, symbol, dict_size, idx, scratch)) {
			fprintf(stderr, "Write error for '%s'\n", destFilePath);
			free(parent);
			free(symbol);
			free(scratch);
			fclose(src);
			fclose(dest);
			return;
		}
	}

	if (ferror(src)) {
		fprintf(stderr, "Read error for '%s'\n", srcFilePath);
	}

	free(parent);
	free(symbol);
	free(scratch);
	fclose(src);
	fclose(dest);
}