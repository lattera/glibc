/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Waiting queues */

typedef struct _pthread_queue pthread_queue;

static inline void queue_init(pthread_queue * q)
{
  q->head = q->tail = NULL;
}

static inline void enqueue(pthread_queue * q, pthread_descr th)
{
  int prio;
  pthread_descr * elt;

  ASSERT(th->p_nextwaiting == NULL);
  if (q->tail == NULL) {
    q->head = th;
    q->tail = th;
    return;
  }
  prio = th->p_priority;
  if (prio > 0) {
    /* Insert in queue according to priority order */
    for (elt = &(q->head); *elt != NULL; elt = &((*elt)->p_nextwaiting)) {
      if (prio > (*elt)->p_priority) {
        th->p_nextwaiting = *elt;
        *elt = th;
        return;
      }
    }
  }
  /* Priority is no greater than any thread in the queue.
     Insert at end of queue */
  q->tail->p_nextwaiting = th;
  q->tail = th;
}

static inline pthread_descr dequeue(pthread_queue * q)
{
  pthread_descr th;
  th = q->head;
  if (th != NULL) {
    q->head = th->p_nextwaiting;
    if (q->head == NULL) q->tail = NULL;
    th->p_nextwaiting = NULL;
  }
  return th;
}
