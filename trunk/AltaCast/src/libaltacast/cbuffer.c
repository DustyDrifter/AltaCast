/* sbuffer.c - jonclegg@yahoo.com
 * general use circular buffer lib
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include "cbuffer.h"

/*********************************************************************************
 * Public functions
 *********************************************************************************/
int	cbuffer_init(CBUFFER *buffer, unsigned long size);
void		cbuffer_destroy(CBUFFER *buffer);
int	cbuffer_extract(CBUFFER *buffer, char *items, unsigned long count);
int	cbuffer_peek(CBUFFER *buffer, char *items, unsigned long count);
int	cbuffer_insert(CBUFFER *buffer, const char *items, unsigned long count);
unsigned long		cbuffer_get_free(CBUFFER *buffer);
unsigned long 		cbuffer_get_used(CBUFFER *buffer);
unsigned long		cbuffer_get_size(CBUFFER *buffer);


/*********************************************************************************
 * Private functions
 *********************************************************************************/
static void	reset(CBUFFER *buffer);
static void	increment(CBUFFER *buffer, unsigned long *index);

int	cbuffer_init2(CBUFFER *buffer, unsigned long size)
{
   if (size == 0) {
		return 0;
   }
   if (size == 0) {
      buffer->size = 0;
      buffer->buf = NULL;
      reset(buffer);
   } else {
      buffer->size = size;
      buffer->buf = (char *)malloc(size);
      reset(buffer);
   }

   return 1;
}

int	cbuffer_init(CBUFFER *buffer, unsigned long size)
{
   if (size == 0) {
		return 0;
   }
   if (size == 0) {
      buffer->size = 0;
      buffer->buf = NULL;
      reset(buffer);
   } else {
      buffer->size = size;
      buffer->buf = (char *)malloc(size);
      reset(buffer);
   }

   pthread_mutex_init(&(buffer->cbuffer_mutex), NULL);

   return 1;
}

void cbuffer_destroy(CBUFFER *buffer)
{
	if (buffer->buf)
	{
		free(buffer->buf);
		buffer->buf = NULL;
	}
}

void cbuffer_clear(CBUFFER *buffer)
{
	pthread_mutex_lock(&(buffer->cbuffer_mutex));
	cbuffer_destroy(buffer);
	cbuffer_init2(buffer, buffer->size);
	pthread_mutex_unlock(&(buffer->cbuffer_mutex));
}
int cbuffer_extract(CBUFFER *buffer, char *items, unsigned long count)
{
	unsigned long i;	
	
	pthread_mutex_lock(&(buffer->cbuffer_mutex));

	if (buffer->buf) {
		for(i = 0; i < count; i++)
		{
			if (buffer->item_count > 0) {
				buffer->item_count--;
			}
			else {
				pthread_mutex_unlock(&(buffer->cbuffer_mutex));
				return BUFFER_EMPTY;
			}

			increment(buffer, &buffer->read_index);
			items[i] = buffer->buf[buffer->read_index];
		}
	}
	else {
		pthread_mutex_unlock(&(buffer->cbuffer_mutex));
		return 0;
	}
	pthread_mutex_unlock(&(buffer->cbuffer_mutex));
	return 1;
}


int cbuffer_peek(CBUFFER *buffer, char *items, unsigned long count)
{
	unsigned long i;	
	unsigned long my_read_index = buffer->read_index;
	unsigned long my_item_count = buffer->item_count;

	for(i = 0; i < count; i++)
	{
		if (my_item_count > 0)
			my_item_count--;
		else
			return BUFFER_EMPTY;

		increment(buffer, &my_read_index);
		items[i] = buffer->buf[my_read_index];
	}
	return 1;
}


int cbuffer_insert(CBUFFER *buffer, const char *items, unsigned long count)
{
	unsigned long i;	
	
	pthread_mutex_lock(&(buffer->cbuffer_mutex));
	for(i = 0; i < count; i++)
	{
		if (buffer->item_count < cbuffer_get_size(buffer)) {
			buffer->item_count++;
		}
		else {
			pthread_mutex_unlock(&(buffer->cbuffer_mutex));
			return BUFFER_FULL;
		}

		increment(buffer, &buffer->write_index);
		buffer->buf[buffer->write_index] = items[i];
	}

	pthread_mutex_unlock(&(buffer->cbuffer_mutex));
	return 1;
}

unsigned long cbuffer_get_size(CBUFFER *buffer)
{
	return buffer->size;

}

unsigned long cbuffer_get_free(CBUFFER *buffer)
{
	return cbuffer_get_size(buffer) - cbuffer_get_used(buffer);
}

unsigned long cbuffer_get_used(CBUFFER *buffer)
{
	return buffer->item_count;
}


void increment(CBUFFER *buffer, unsigned long *index)
{
   (*index)++;
   if (*index >= cbuffer_get_size(buffer))
      *index = 0;
}

void reset(CBUFFER *buffer)
{
	buffer->read_index = buffer->write_index = cbuffer_get_size(buffer) - 1;
	buffer->item_count = 0;
}


