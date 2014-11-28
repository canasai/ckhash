/*
 * Copyright (C) 2008-2009 National Institute of Information and Communications Technology
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include "cuckoo_hash.h"

#define TRUE 1
#define FALSE !TRUE
#define MAX_FUNCTION_SIZE 128
#define CKH_MAX_DECIMAL_FORMAT 64
#define MALLOC(type, n) (type *)malloc((n) * sizeof(type))
#define CALLOC(type, n) (type *)calloc((n), sizeof(type))
#define REALLOC(data, type, n) (type *)realloc(data, n * sizeof(type))
#define KEY_CMP(s1, s2) strcmp((const char *)s1, (const char *)s2)
#define EXIT_IF(expr) \
	do { \
		if (expr) { \
			fprintf(stderr, "\n%s:%d: In function `%s': %s\n\n", \
				__FILE__, __LINE__, __FUNCTION__, \
				strerror(errno)); \
			fflush(stderr); \
			exit(EXIT_FAILURE); \
		} \
	} while (0)
#define EXIT_HOW_IF(expr, where, fmt, ...) \
	do { \
		if (expr) { \
			if (where) \
				fprintf(stderr, "\n%s:%d: In function `%s': ", \
					__FILE__, __LINE__, __FUNCTION__); \
			else \
				fprintf(stderr, "\n"); \
			fprintf(stderr, fmt, ##__VA_ARGS__); \
			fprintf(stderr, "\n\n"); \
			fflush(stderr); \
			exit(EXIT_FAILURE); \
		} \
	} while (0)

inline void ckh_init(int a[], int function_size)
{
	int i;

	for (i = 0; i < function_size; i++)
		a[i] = ((int)rand() << 1) + 1;
}

inline void ckh_hash(unsigned long *h, int a[], int function_size,
		     int table_size, int shift, const unsigned char *key)
{
	int i;

	*h = 0;
	for (i = 0; key[i]; i++)
		*h ^= (unsigned int)(a[(i % function_size)] * key[i]);
	*h = ((unsigned int)*h >> shift) % table_size;
}

CKHash_Table *ckh_alloc_table(int table_size)
{
	CKHash_Table *D = CALLOC(CKHash_Table, 1);
	EXIT_IF(D == NULL);
	D->size = 0;
	D->table_size = table_size;
	D->mean_size = 5 * (2 * table_size) / 12;
	D->min_size = (2 * table_size) / 5;
	D->shift = 32 - (int)(log(table_size) / log(2) + 0.5);
	D->max_chain = 4 + (int)(4 * log(table_size) / log(2) + 0.5);
	D->T1 = CALLOC(CKHash_Cell, D->table_size);
	EXIT_IF(D->T1 == NULL);
	D->T2 = CALLOC(CKHash_Cell, D->table_size);
	EXIT_IF(D->T2 == NULL);
	D->function_size = MAX_FUNCTION_SIZE;
	D->a1 = MALLOC(int, D->function_size);
	EXIT_IF(D->a1 == NULL);
	D->a2 = MALLOC(int, D->function_size);
	EXIT_IF(D->a2 == NULL);
	ckh_init(D->a1, D->function_size);
	ckh_init(D->a2, D->function_size);

	return D;
}

CKHash_Table *ckh_construct_table(int min_size)
{
	srand(time(NULL));

	return ckh_alloc_table(min_size);
}

int ckh_rehash_insert(CKHash_Table *D, unsigned char *key, int value)
{
	unsigned long hkey;
	unsigned int j;
	CKHash_Cell x, temp;

	x.key = key;
	x.value = value;

	for (j = 0; j < D->max_chain; j++) {
		ckh_hash(&hkey, D->a1, D->function_size, D->table_size,
			 D->shift, x.key);
		temp = D->T1[hkey];
		D->T1[hkey] = x;
		if (temp.key == NULL)
			return TRUE;

		x = temp;
		ckh_hash(&hkey, D->a2, D->function_size, D->table_size,
			 D->shift, x.key);
		temp = D->T2[hkey];
		D->T2[hkey] = x;
		if (temp.key == NULL)
			return TRUE;

		x = temp;
	}

	for (j = 0; j < D->table_size; j++) {
		D->T1[j].key = D->T2[j].key = NULL;
		D->T1[j].value = D->T2[j].value = 0;
	}

	ckh_init(D->a1, D->function_size);
	ckh_init(D->a2, D->function_size);

	return FALSE;
}

void ckh_rehash(CKHash_Table *D, int new_size)
{
	CKHash_Table *D_new;
	unsigned int k;

	D_new = ckh_alloc_table(new_size);

	for (k = 0; k < D->table_size; k++) {
		if ((D->T1[k].key != NULL) &&
		    (!ckh_rehash_insert(D_new, D->T1[k].key, D->T1[k].value))) {
			k = -1;
			continue;
		}
		if ((D->T2[k].key != NULL) &&
		    (!ckh_rehash_insert(D_new, D->T2[k].key, D->T2[k].value)))
			k = -1;
	}

	free(D->T1);
	free(D->T2);
	free(D->a1);
	free(D->a2);

	D_new->size = D->size;
	*D = *D_new;
	free(D_new);
}

int ckh_insert(CKHash_Table *D, const unsigned char *key, int value)
{
	unsigned long h1, h2;
	unsigned int j;
	CKHash_Cell x, temp;

	/*
	 * If the element is already in D, then overwrite and return.
	 */
	ckh_hash(&h1, D->a1, D->function_size, D->table_size, D->shift, key);
	if (D->T1[h1].key != NULL && KEY_CMP(D->T1[h1].key, key) == 0) {
		D->T1[h1].value = value;
		return FALSE;
	}

	ckh_hash(&h2, D->a2, D->function_size, D->table_size, D->shift, key);
	if (D->T2[h2].key != NULL && KEY_CMP(D->T2[h2].key, key) == 0) {
		D->T2[h2].value = value;
		return FALSE;
	}

	/*
	 * If not, the insert the new element in D.
	 */
	int key_len = strlen((const char *)key) + 1;
	x.key = MALLOC(unsigned char, key_len);
	EXIT_IF(x.key == NULL);

	strncpy((char *)x.key, (const char *)key, key_len);
	x.value = value;

	for (j = 0; j < D->max_chain; j++) {
		temp = D->T1[h1];
		D->T1[h1] = x;
		if (temp.key == NULL) {
			D->size++;
			if (D->table_size < D->size)
				ckh_rehash(D, 2 * D->table_size);
			return TRUE;
		}

		x = temp;
		ckh_hash(&h2, D->a2, D->function_size, D->table_size, D->shift,
			 x.key);
		temp = D->T2[h2];
		D->T2[h2] = x;
		if (temp.key == NULL) {
			D->size++;
			if (D->table_size < D->size)
				ckh_rehash(D, 2 * D->table_size);
			return TRUE;
		}

		x = temp;
		ckh_hash(&h1, D->a1, D->function_size, D->table_size, D->shift,
			 x.key);
	}

	/*
	 * Forced rehash.
	 */
	if (D->size < D->mean_size)
		ckh_rehash(D, D->table_size);
	else
		ckh_rehash(D, 2 * D->table_size);

	ckh_insert(D, x.key, x.value);
	free(x.key); /* Free x.key, because it is already copied. */

	return TRUE;
}

int ckh_lookup(CKHash_Table *D, const unsigned char *key)
{
	unsigned long hkey;

	ckh_hash(&hkey, D->a1, D->function_size, D->table_size, D->shift, key);
	if (D->T1[hkey].key != NULL && KEY_CMP(D->T1[hkey].key, key) == 0)
		return TRUE;

	ckh_hash(&hkey, D->a2, D->function_size, D->table_size, D->shift, key);
	if (D->T2[hkey].key != NULL && KEY_CMP(D->T2[hkey].key, key) == 0)
		return TRUE;

	return FALSE;
}

int ckh_delete(CKHash_Table *D, const unsigned char *key)
{
	unsigned long hkey;

	ckh_hash(&hkey, D->a1, D->function_size, D->table_size, D->shift, key);
	if (D->T1[hkey].key != NULL) {
		if (KEY_CMP(D->T1[hkey].key, key) == 0) {
			free(D->T1[hkey].key);
			D->T1[hkey].key = NULL;
			D->size--;
			if (D->size < D->min_size)
				ckh_rehash(D, D->table_size / 2);
			return TRUE;
		}
	}

	ckh_hash(&hkey, D->a2, D->function_size, D->table_size, D->shift, key);
	if (D->T2[hkey].key != NULL) {
		if (KEY_CMP(D->T2[hkey].key, key) == 0) {
			free(D->T2[hkey].key);
			D->T2[hkey].key = NULL;
			D->size--;
			if (D->size < D->min_size)
				ckh_rehash(D, D->table_size / 2);
			return TRUE;
		}
	}

	return FALSE;
}

int ckh_get(CKHash_Table *D, const unsigned char *key, int *ret_value)
{
	unsigned long hkey;
	int found = FALSE;

	ckh_hash(&hkey, D->a1, D->function_size, D->table_size, D->shift, key);
	if (D->T1[hkey].key != NULL && KEY_CMP(D->T1[hkey].key, key) == 0) {
		*ret_value = D->T1[hkey].value;
		found = TRUE;
	}

	ckh_hash(&hkey, D->a2, D->function_size, D->table_size, D->shift, key);
	if (D->T2[hkey].key != NULL && KEY_CMP(D->T2[hkey].key, key) == 0) {
		*ret_value = D->T2[hkey].value;
		found = TRUE;
	}

	return found;
}

int ckh_increase_value(CKHash_Table *D, const unsigned char *key)
{
	unsigned long hkey;

	ckh_hash(&hkey, D->a1, D->function_size, D->table_size, D->shift, key);
	if (D->T1[hkey].key != NULL) {
		if (KEY_CMP(D->T1[hkey].key, key) == 0) {
			D->T1[hkey].value++;
			return TRUE;
		}
	}

	ckh_hash(&hkey, D->a2, D->function_size, D->table_size, D->shift, key);
	if (D->T2[hkey].key != NULL) {
		if (KEY_CMP(D->T2[hkey].key, key) == 0) {
			D->T2[hkey].value++;
			return TRUE;
		}
	}

	return FALSE;
}

int ckh_increase_value2(CKHash_Table *D, const unsigned char *key, int value)
{
	unsigned long hkey;

	ckh_hash(&hkey, D->a1, D->function_size, D->table_size, D->shift, key);
	if (D->T1[hkey].key != NULL) {
		if (KEY_CMP(D->T1[hkey].key, key) == 0) {
			D->T1[hkey].value += value;
			return TRUE;
		}
	}

	ckh_hash(&hkey, D->a2, D->function_size, D->table_size, D->shift, key);
	if (D->T2[hkey].key != NULL) {
		if (KEY_CMP(D->T2[hkey].key, key) == 0) {
			D->T2[hkey].value += value;
			return TRUE;
		}
	}

	return FALSE;
}

int ckh_decrease_value(CKHash_Table *D, const unsigned char *key)
{
	unsigned long hkey;

	ckh_hash(&hkey, D->a1, D->function_size, D->table_size, D->shift, key);
	if (D->T1[hkey].key != NULL) {
		if (KEY_CMP(D->T1[hkey].key, key) == 0) {
			D->T1[hkey].value--;
			return TRUE;
		}
	}

	ckh_hash(&hkey, D->a2, D->function_size, D->table_size, D->shift, key);
	if (D->T2[hkey].key != NULL) {
		if (KEY_CMP(D->T2[hkey].key, key) == 0) {
			D->T2[hkey].value--;
			return TRUE;
		}
	}

	return FALSE;
}

CKHash_Table *ckh_destruct_table(CKHash_Table *D)
{
	if (D == NULL)
		return NULL;

	unsigned int j;

	for (j = 0; j < D->table_size; j++) {
		if (D->T1[j].key != NULL)
			free(D->T1[j].key);
		if (D->T2[j].key != NULL)
			free(D->T2[j].key);
	}

	free(D->T1);
	free(D->T2);
	free(D->a1);
	free(D->a2);
	free(D);

	return NULL;
}

void ckh_print(CKHash_Table *D)
{
	unsigned int i;

	for (i = 0; i < D->table_size; i++) {
		if (D->T1[i].key != NULL)
			printf("%s\t%d\n", D->T1[i].key, D->T1[i].value);

		if (D->T2[i].key != NULL)
			printf("%s\t%d\n", D->T2[i].key, D->T2[i].value);
	}
}

static int key_cmp(const void *a_, const void *b_)
{
	const char **a = (const char **)a_;
	const char **b = (const char **)b_;

	return strcmp(*a, *b);
}

int ckh_export_table(CKHash_Table *D, const char *file_name, const char *delimiter)
{
	unsigned int i, j;

 	char **key = CALLOC(char *, D->size);
 	j = 0;

	for (i = 0; i < D->table_size; i++) {
		if (D->T1[i].key != NULL)
			key[j++] =  (char *)D->T1[i].key;
		if (D->T2[i].key != NULL)
			key[j++] =  (char *)D->T2[i].key;
	}

	EXIT_HOW_IF(j != D->size, 1, "Element size differs: read=%d expected=%d",
		    (int)j, (int)D->size);

	qsort(key, j, sizeof(char *), key_cmp);
	FILE *file_ptr = fopen(file_name, "w");

	for (i = 0; i < j; i++) {
		int value;
		EXIT_HOW_IF(!ckh_get(D, (const unsigned char *)key[i], &value), 1,
			    "Cannot find key %s", key[i]);
		fprintf(file_ptr, "%s%s%d\n", key[i], delimiter, value);
	}

	fclose(file_ptr);
	free(key);

	return TRUE;
}

int ckh_export_key(CKHash_Table *D, const char *file_name)
{
	unsigned int i, j;

 	char **key = CALLOC(char *, D->size);
 	j = 0;

	for (i = 0; i < D->table_size; i++) {
		if (D->T1[i].key != NULL)
			key[j++] =  (char *)D->T1[i].key;
		if (D->T2[i].key != NULL)
			key[j++] =  (char *)D->T2[i].key;
	}

	EXIT_HOW_IF(j != D->size, 1, "Element size differs: read=%d expected=%d",
		    (int)j, (int)D->size);

	qsort(key, j, sizeof(char *), key_cmp);
	FILE *file_ptr = fopen(file_name, "w");

	for (i = 0; i < j; i++)
		fprintf(file_ptr, "%s\n", key[i]);

	fclose(file_ptr);
	free(key);

	return TRUE;
}
