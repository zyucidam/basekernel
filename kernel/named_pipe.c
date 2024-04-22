/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "named_pipe.h"
#include "kmalloc.h"
#include "process.h"
#include "page.h"
#include "hash_set.h"



/*use hash map to maintain named pipes*/
static struct hash_set_node *named_pipe_node[MAX_NAMED_PIPES_NUM] = {};
static struct hash_set named_pipe_list = {
	.total_buckets = MAX_NAMED_PIPES_NUM,
	.num_entries = 0,
	.head = named_pipe_node
};

struct named_pipe *named_pipe_create(const char* fname)
{
	/*avoid named pipe bucket overflow*/
	if (named_pipe_list.num_entries >= MAX_NAMED_PIPES_NUM) return 1;
	struct named_pipe *np = kmalloc(sizeof(struct named_pipe));
	
	np->fname = fname;
	np->buffer =  page_alloc(1);
	if(!np->buffer) {
		kfree(np);
		return 0;
	}

	hash_set_add(&named_pipe_list, hash_string(fname, 0, MAX_NAMED_PIPES_NUM), np);
	np->read_pos = 0;
	np->write_pos = 0;
	np->flushed = 0;
	np->queue.head = 0;
	np->queue.tail = 0;
	np->refcount = 1;
	return np;
}

/*add refcount for a named_pipe*/
struct named_pipe *named_pipe_addref( struct named_pipe *np )
{
	np->refcount++;
	return np;
}
/*look up named pipe list by hash lookup*/
struct named_pipe *named_pipe_lookup(const char* fname) {
	struct named_pipe *np = (struct named_pipe *)hash_set_lookup(&named_pipe_list, hash_string(fname, 0, MAX_NAMED_PIPES_NUM));
	
    return np ? named_pipe_addref(np) : 0;
}
/*flush named pipe/*/
void named_pipe_flush(struct named_pipe *np)
{
	if(np) {
		np->flushed = 1;
	}
}
/*delete named pipe*/
void named_pipe_delete(struct named_pipe *np)
{
	if(!np) return;

	np->refcount--;
	if(np->refcount==0) {
		if(np->buffer) {
			hash_set_remove(&named_pipe_list, hash_string(np->fname, 0, MAX_NAMED_PIPES_NUM));
			page_free(np->buffer);
		}
		kfree(np);
	}
}
/*write message into named pipe buffer*/
/*
	sender: 
	1. write from buffer to np->buffer until np->write_pos == np->read_pos 
	2. process_wakeup_all() 
	3. return written
	*/
static int named_pipe_write_internal(struct named_pipe *np, char *buffer, int size, int blocking )
{
	if(!np || !buffer) {
		return -1;
	}
	if(np->flushed) return 0;
	
	

	int written = 0;
	for(written; written < size; written++) {
		while((np->write_pos + 1) % PIPE_SIZE == np->read_pos) {
			if(np->flushed) {
				np->flushed = 0;
				return written;
			}
			process_wait(&np->queue);
			np->flushed = 1;
		}
		
		np->buffer[np->write_pos] = buffer[written];
		np->write_pos = (np->write_pos + 1) % PIPE_SIZE;
		
	}


	if(blocking) {
		process_wakeup_all(&np->queue);
	}
	np->flushed = 0;
	return written;
}
/*wrtie with block*/
int named_pipe_write(struct named_pipe *np, char *buffer, int size)
{
	return named_pipe_write_internal(np, buffer, size, 1);
}
/*wrtie with nonblock*/
int named_pipe_write_nonblock(struct named_pipe *np, char *buffer, int size)
{
	return named_pipe_write_internal(np, buffer, size, 0);
}
/*read message from named pipe buffer*/
/*
	receiver: 
	1. must be np->write_pos == np->read_pos 
	2. process_wait
	3. process_wakeup_all() from named_pipe_write_internal
	4. read from np->buffer to buffer until np->write_pos == np->read_pos 
	5. in order to avoid receiver process_wait() again, after first time of process_wait() set flushed = 1
	6. return read
	*/
static int named_pipe_read_internal(struct named_pipe *np, char *buffer, int size, int blocking)
{
	if(!np || !buffer) {
		return -1;
	}

	if(np->flushed) return 0;
	
	
	int read = 0;
	for(read = 0; read < size; read++) {
		while(np->write_pos == np->read_pos) {
			if(np->flushed) {
				np->flushed = 0;
				return read;
			}				
			if (blocking == 0) {
				return -1;
			}

			process_wait(&np->queue);
			np->flushed = 1;
			}
		buffer[read] = np->buffer[np->read_pos];
		np->read_pos = (np->read_pos + 1) % PIPE_SIZE;
	}
		
		
	if(blocking) { 
		process_wakeup_all(&np->queue);		
		}
	
	np->flushed = 0;
	return read;
}
/*read with block*/
int named_pipe_read(struct named_pipe *np, char *buffer, int size)
{
	return named_pipe_read_internal(np, buffer, size, 1);
}
/*read with nonblock*/
int named_pipe_read_nonblock(struct named_pipe *np, char *buffer, int size)
{
	return named_pipe_read_internal(np, buffer, size, 0);
}
/*get name pipe size*/
int named_pipe_size( struct named_pipe *np )
{
	if(!np) return -1;
	return (np->write_pos - np->read_pos + PIPE_SIZE) % PIPE_SIZE;
}

