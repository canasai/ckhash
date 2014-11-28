/*
 * Copyright (C) 2008-2009 National Institute of Information and Communications Technology
 */

#ifndef __cuckoo_hash_H
#define __cuckoo_hash_H

typedef struct {			/* Hash table cell type */
	unsigned char *key;		/* Hash key */
	int value;			/* Hash value */
} CKHash_Cell;

typedef struct {
	unsigned int size;		/*  Current size */
	unsigned int table_size;	/*  Size of hash tables */
	int shift;			/*  Value used for hash function */
	unsigned int min_size;		/*  Rehash trigger size */
	unsigned int mean_size;		/*  Rehash trigger size */
	unsigned int max_chain;		/*  Max iterations in insertion */
	CKHash_Cell *T1;		/*  Pointer to hash table 1 */
	CKHash_Cell *T2;		/*  Pointer to hash table 2 */
	int function_size;		/*  Size of hash function */
	int *a1;			/*  Hash function 1 */
	int *a2;			/*  Hash function 2 */
} CKHash_Table;

CKHash_Table *ckh_alloc_table(int table_size);

CKHash_Table *ckh_construct_table(int min_size);

int ckh_insert(CKHash_Table *D, const unsigned char *key, int value);

int ckh_lookup(CKHash_Table *D, const unsigned char *key);

int ckh_delete(CKHash_Table *D, const unsigned char *key);

int ckh_get(CKHash_Table *D, const unsigned char *key, int *ret_value);

int ckh_increase_value(CKHash_Table *D, const unsigned char *key);

int ckh_increase_value2(CKHash_Table *D, const unsigned char *key, int value);

int ckh_decrease_value(CKHash_Table *D, const unsigned char *key);

CKHash_Table *ckh_destruct_table(CKHash_Table *D);

void ckh_print(CKHash_Table *D);

int ckh_export_table(CKHash_Table *D, const char *file_name, const char *delimiter);

int ckh_export_key(CKHash_Table *D, const char *file_name);

#endif /* __cuckoo_hash_H */
