/*
 * Beat up the pthread_key_create and pthread_key_delete
 * functions.
 */

#if 0
#define CHATTY
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

const int beatup_iterations = 10000;
const int num_threads = 30;
const int max_keys = 500;

struct key_list {
  struct key_list *next;
  pthread_key_t key;	
};

struct key_list *key_list;
pthread_mutex_t key_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Create a new key and put it at the tail of a linked list.
 * If the linked list grows to a certain length, delete a key from the
 * head of * the list. 
 */

static void 
beat_up(void)
{
  struct key_list *new = malloc(sizeof *new);
  struct key_list **iter, *old_key = 0;
  int key_count = 0;

  if (new == 0) {
    fprintf(stderr, "malloc failed\n");
    abort();
  }

  new->next = 0;

  if (pthread_key_create(&new->key, 0) != 0) {
    fprintf(stderr, "pthread_key_create failed\n");
    abort();
  }

  if (pthread_getspecific(new->key) != 0) {
    fprintf(stderr, "new pthread_key_t resolves to non-null value\n");
    abort();
  }

  pthread_setspecific(new->key, (void *) 1);

#ifdef CHATTY
  printf("created key\n");
#endif

  pthread_mutex_lock(&key_lock);

  for (iter = &key_list; *iter != 0; iter = &(*iter)->next)
    key_count++;

  *iter = new;

  if (key_count > max_keys) {
    old_key = key_list;
    key_list = key_list->next;
  }

  pthread_mutex_unlock(&key_lock);

  if (old_key != 0) {
#ifdef CHATTY
    printf("deleting key\n");
#endif
    pthread_key_delete(old_key->key);
  }
}

static void *
thread(void *arg)
{
  int i;
  for (i = 0; i < beatup_iterations; i++) 
    beat_up();
  return 0;
}

int
main(void)
{
  int i;
  pthread_attr_t detached_thread;

  pthread_attr_init(&detached_thread);
  pthread_attr_setdetachstate(&detached_thread, PTHREAD_CREATE_DETACHED);

  for (i = 0; i < num_threads; i++) {
    pthread_t thread_id;
    while (pthread_create(&thread_id, &detached_thread, thread, 0) == EAGAIN) {
      /* let some threads die, so system can breathe. :) */
      sleep(1);
    }
  }

  pthread_exit(0);
}
