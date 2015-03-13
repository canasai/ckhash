#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cuckoo_hash.h"

#define MIN_TAB_SIZE (4)
#define MAX_LINE_LEN (8192)

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	char line[MAX_LINE_LEN];
	CKHash_Table *D = ckh_construct_table(MIN_TAB_SIZE);
	FILE *file_ptr = fopen(argv[1], "r");
	int i = 0;

	printf("test insert...\n");
	while (fgets(line, MAX_LINE_LEN, file_ptr) != NULL) {
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		ckh_insert(D, (unsigned char *)line, i++);
	}

	printf(" %d entries, table size = %d\n", D->size, D->table_size);
	rewind(file_ptr);

	int err = 0;
	printf("test lookup...\n");
	while (fgets(line, MAX_LINE_LEN, file_ptr) != NULL) {
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		if (!ckh_lookup(D, (unsigned char *)line))
			err++;
	}

	printf(" %d errors\n", err);
	rewind(file_ptr);

	printf("test get...\n");
	char *w[3] = {"ALGOL", "ANSI", "ansi"};
	int ret_value = 0;

	for (i = 0; i < 3; i++) {
		if (ckh_get(D, (unsigned char *)w[i], &ret_value))
			printf(" found [%s] value=%d\n", w[i], ret_value);
		else
			printf(" could not find [%s]\n", w[i]);
	}

	err = 0;
	printf("test delete...\n");
	while (fgets(line, MAX_LINE_LEN, file_ptr) != NULL) {
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		if (!ckh_delete(D, (unsigned char *)line))
			err++;
	}

	printf(" %d errors, remain %d entries, table size = %d\n",
	       err, D->size, D->table_size);
	fclose(file_ptr);

	ckh_destruct_table(D);

	return EXIT_SUCCESS;
}
