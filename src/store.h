
#ifndef __STORE_H__
#define __STORE_H__

#include <sys/time.h>
#include "hashtable.h"

struct ll_stat;
typedef struct ll_stat *StatNode;

struct store_key {
	char *namespace;
	char *key;
};

struct store_value {
	char *namespace;
	char *key;
	int count;
	StatNode stats;
};

static unsigned int hash_from_key(void *ky);
static int equal_keys(void *k1, void *k2);

struct hashtable *add_stat(struct hashtable *h, char *namespace, char *key, int value);

StatNode last_for_nsk(struct hashtable *hash_table, char *namespace, char *key);
StatNode create_stat(int value, time_t when);
StatNode push_stat(StatNode head, int value, time_t when);

#endif