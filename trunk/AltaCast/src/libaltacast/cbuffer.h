#ifndef __CBUFFER_H__
#define __CBUFFER_H__

#ifdef _DMALLOC_
#include <dmalloc.h>
#endif

#include <pthread.h>
typedef struct CBUFFERst
{
	pthread_mutex_t cbuffer_mutex;
	char	*buf;
	unsigned long	size;
	unsigned long	write_index;
	unsigned long	read_index;
	unsigned long	item_count;
} CBUFFER;

#define BUFFER_EMPTY	3
#define BUFFER_FULL	2
#ifdef __cplusplus
extern "C" {
#endif
int	cbuffer_init(CBUFFER *buffer, unsigned long size);
void			cbuffer_destroy(CBUFFER *buffer);
int	cbuffer_extract(CBUFFER *buffer, char *items, unsigned long count);
int	cbuffer_peek(CBUFFER *buffer, char *items, unsigned long count);
int	cbuffer_insert(CBUFFER *buffer, const char *items, unsigned long count);
unsigned long		cbuffer_get_free(CBUFFER *buffer);
unsigned long 		cbuffer_get_used(CBUFFER *buffer);
unsigned long		cbuffer_get_size(CBUFFER *buffer);
void cbuffer_clear(CBUFFER *buffer);
#ifdef __cplusplus
}
#endif

#endif //__CBUFFER_H__

