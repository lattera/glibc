/* Extended resolver state separate from struct __res_state.
   Copyright (C) 2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <resolv_conf.h>

#include <alloc_buffer.h>
#include <assert.h>
#include <libc-lock.h>
#include <resolv-internal.h>

/* _res._u._ext.__glibc_extension_index is used as an index into a
   struct resolv_conf_array object.  The intent of this construction
   is to make reasonably sure that even if struct __res_state objects
   are copied around and patched by applications, we can still detect
   accesses to stale extended resolver state.  */
#define DYNARRAY_STRUCT resolv_conf_array
#define DYNARRAY_ELEMENT struct resolv_conf *
#define DYNARRAY_PREFIX resolv_conf_array_
#define DYNARRAY_INITIAL_SIZE 0
#include <malloc/dynarray-skeleton.c>

/* A magic constant for XORing the extension index
   (_res._u._ext.__glibc_extension_index).  This makes it less likely
   that a valid index is created by accident.  In particular, a zero
   value leads to an invalid index.  */
#define INDEX_MAGIC 0x26a8fa5e48af8061ULL

/* Global resolv.conf-related state.  */
struct resolv_conf_global
{
  /* struct __res_state objects contain the extension index
     (_res._u._ext.__glibc_extension_index ^ INDEX_MAGIC), which
     refers to an element of this array.  When a struct resolv_conf
     object (extended resolver state) is associated with a struct
     __res_state object (legacy resolver state), its reference count
     is increased and added to this array.  Conversely, if the
     extended state is detached from the basic state (during
     reinitialization or deallocation), the index is decremented, and
     the array element is overwritten with NULL.  */
  struct resolv_conf_array array;

};

/* Lazily allocated storage for struct resolv_conf_global.  */
static struct resolv_conf_global *global;

/* The lock synchronizes access to global and *global.  It also
   protects the __refcount member of struct resolv_conf.  */
__libc_lock_define_initialized (static, lock);

/* Ensure that GLOBAL is allocated and lock it.  Return NULL if
   memory allocation failes.  */
static struct resolv_conf_global *
get_locked_global (void)
{
  __libc_lock_lock (lock);
  /* Use relaxed MO through because of load outside the lock in
     __resolv_conf_detach.  */
  struct resolv_conf_global *global_copy = atomic_load_relaxed (&global);
  if (global_copy == NULL)
    {
      global_copy = calloc (1, sizeof (*global));
      if (global_copy == NULL)
        return NULL;
      atomic_store_relaxed (&global, global_copy);
      resolv_conf_array_init (&global_copy->array);
    }
  return global_copy;
}

/* Relinquish the lock acquired by get_locked_global.  */
static void
put_locked_global (struct resolv_conf_global *global_copy)
{
  __libc_lock_unlock (lock);
}

/* Decrement the reference counter.  The caller must acquire the lock
   around the function call.  */
static void
conf_decrement (struct resolv_conf *conf)
{
  assert (conf->__refcount > 0);
  if (--conf->__refcount == 0)
    free (conf);
}

/* Internal implementation of __resolv_conf_get, without validation
   against *RESP.  */
static struct resolv_conf *
resolv_conf_get_1 (const struct __res_state *resp)
{
  /* Not initialized, and therefore no assoicated context.  */
  if (!(resp->options & RES_INIT))
    return NULL;

  struct resolv_conf_global *global_copy = get_locked_global ();
  if (global_copy == NULL)
    /* A memory allocation failure here means that no associated
       contexts exists, so returning NULL is correct.  */
    return NULL;
  size_t index = resp->_u._ext.__glibc_extension_index ^ INDEX_MAGIC;
  struct resolv_conf *conf;
  if (index < resolv_conf_array_size (&global_copy->array))
    {
      conf = *resolv_conf_array_at (&global_copy->array, index);
      assert (conf->__refcount > 0);
      ++conf->__refcount;
    }
  else
    conf = NULL;
  put_locked_global (global_copy);
  return conf;
}

/* Check that *RESP and CONF match.  Used by __resolv_conf_get.  */
static bool
resolv_conf_matches (const struct __res_state *resp,
                     const struct resolv_conf *conf)
{
  /* Check that the search list in *RESP has not been modified by the
     application.  */
  {
    if (!(resp->dnsrch[0] == resp->defdname
          && resp->dnsrch[MAXDNSRCH] == NULL))
      return false;
    size_t search_list_size = 0;
    for (size_t i = 0; i < conf->search_list_size; ++i)
      {
        if (resp->dnsrch[i] != NULL)
          {
            search_list_size += strlen (resp->dnsrch[i]) + 1;
            if (strcmp (resp->dnsrch[i], conf->search_list[i]) != 0)
              return false;
          }
        else
          {
            /* resp->dnsrch is truncated if the number of elements
               exceeds MAXDNSRCH, or if the combined storage space for
               the search list exceeds what can be stored in
               resp->defdname.  */
            if (i == MAXDNSRCH || search_list_size > sizeof (resp->dnsrch))
              break;
            /* Otherwise, a mismatch indicates a match failure.  */
            return false;
          }
      }
  }
  return true;
}

struct resolv_conf *
__resolv_conf_get (struct __res_state *resp)
{
  struct resolv_conf *conf = resolv_conf_get_1 (resp);
  if (conf == NULL)
    return NULL;
  if (resolv_conf_matches (resp, conf))
    return conf;
  __resolv_conf_put (conf);
  return NULL;
}

void
__resolv_conf_put (struct resolv_conf *conf)
{
  if (conf == NULL)
    return;

  __libc_lock_lock (lock);
  conf_decrement (conf);
  __libc_lock_unlock (lock);
}

struct resolv_conf *
__resolv_conf_allocate (const struct resolv_conf *init)
{
  /* Space needed by the strings.  */
  size_t string_space = 0;
  for (size_t i = 0; i < init->search_list_size; ++i)
    string_space += strlen (init->search_list[i]) + 1;

  /* Allocate the buffer.  */
  void *ptr;
  struct alloc_buffer buffer = alloc_buffer_allocate
    (sizeof (struct resolv_conf)
     + init->search_list_size * sizeof (init->search_list[0])
     + string_space,
     &ptr);
  struct resolv_conf *conf
    = alloc_buffer_alloc (&buffer, struct resolv_conf);
  if (conf == NULL)
    /* Memory allocation failure.  */
    return NULL;
  assert (conf == ptr);

  /* Initialize the contents.  */
  conf->__refcount = 1;
  conf->initstamp = __res_initstamp;

  /* Allocate and fill the search list array.  */
  {
    conf->search_list_size = init->search_list_size;
    const char **array = alloc_buffer_alloc_array
      (&buffer, const char *, init->search_list_size);
    conf->search_list = array;
    for (size_t i = 0; i < init->search_list_size; ++i)
      array[i] = alloc_buffer_copy_string (&buffer, init->search_list[i]);
  }

  assert (!alloc_buffer_has_failed (&buffer));
  return conf;
}

/* Update *RESP from the extended state.  */
static __attribute__ ((nonnull (1, 2), warn_unused_result)) bool
update_from_conf (struct __res_state *resp, const struct resolv_conf *conf)
{
  /* Fill in the prefix of the search list.  It is truncated either at
     MAXDNSRCH, or if reps->defdname has insufficient space.  */
  {
    struct alloc_buffer buffer
      = alloc_buffer_create (resp->defdname, sizeof (resp->defdname));
    size_t size = conf->search_list_size;
    size_t i;
    for (i = 0; i < size && i < MAXDNSRCH; ++i)
      {
        resp->dnsrch[i] = alloc_buffer_copy_string
          (&buffer, conf->search_list[i]);
        if (resp->dnsrch[i] == NULL)
          /* No more space in resp->defdname.  Truncate.  */
          break;
      }
    resp->dnsrch[i] = NULL;
  }

  /* The overlapping parts of both configurations should agree after
     initialization.  */
  assert (resolv_conf_matches (resp, conf));
  return true;
}

/* Decrement the configuration object at INDEX and free it if the
   reference counter reaches 0.  *GLOBAL_COPY must be locked and
   remains so.  */
static void
decrement_at_index (struct resolv_conf_global *global_copy, size_t index)
{
  if (index < resolv_conf_array_size (&global_copy->array))
    {
      /* Index found.  Deallocate the struct resolv_conf object once
         the reference counter reaches.  Free the array slot.  */
      struct resolv_conf **slot
        = resolv_conf_array_at (&global_copy->array, index);
      struct resolv_conf *conf = *slot;
      if (conf != NULL)
        {
          conf_decrement (conf);
          /* Clear the slot even if the reference count is positive.
             Slots are not shared.  */
          *slot = NULL;
        }
    }
}

bool
__resolv_conf_attach (struct __res_state *resp, struct resolv_conf *conf)
{
  assert (conf->__refcount > 0);

  struct resolv_conf_global *global_copy = get_locked_global ();
  if (global_copy == NULL)
    {
      free (conf);
      return false;
    }

  /* Try to find an unused index in the array.  */
  size_t index;
  {
    size_t size = resolv_conf_array_size (&global_copy->array);
    bool found = false;
    for (index = 0; index < size; ++index)
      {
        struct resolv_conf **p
          = resolv_conf_array_at (&global_copy->array, index);
        if (*p == NULL)
          {
            *p = conf;
            found = true;
            break;
          }
      }

    if (!found)
      {
        /* No usable index found.  Increase the array size.  */
        resolv_conf_array_add (&global_copy->array, conf);
        if (resolv_conf_array_has_failed (&global_copy->array))
          {
            put_locked_global (global_copy);
            __set_errno (ENOMEM);
            return false;
          }
        /* The new array element was added at the end.  */
        index = size;
      }
  }

  /* We have added a new reference to the object.  */
  ++conf->__refcount;
  assert (conf->__refcount > 0);
  put_locked_global (global_copy);

  if (!update_from_conf (resp, conf))
    {
      /* Drop the reference we acquired.  Reacquire the lock.  The
         object has already been allocated, so it cannot be NULL this
         time.  */
      global_copy = get_locked_global ();
      decrement_at_index (global_copy, index);
      put_locked_global (global_copy);
      return false;
    }
  resp->_u._ext.__glibc_extension_index = index ^ INDEX_MAGIC;

  return true;
}

void
__resolv_conf_detach (struct __res_state *resp)
{
  if (atomic_load_relaxed (&global) == NULL)
    /* Detach operation after a shutdown, or without any prior
       attachment.  We cannot free the data (and there might not be
       anything to free anyway).  */
    return;

  struct resolv_conf_global *global_copy = get_locked_global ();
  size_t index = resp->_u._ext.__glibc_extension_index ^ INDEX_MAGIC;
  decrement_at_index (global_copy, index);

  /* Clear the index field, so that accidental reuse is less
     likely.  */
  resp->_u._ext.__glibc_extension_index = 0;

  put_locked_global (global_copy);
}

/* Deallocate the global data.  */
static void __attribute__ ((section ("__libc_thread_freeres_fn")))
freeres (void)
{
  /* No locking because this function is supposed to be called when
     the process has turned single-threaded.  */
  if (global == NULL)
    return;

  /* Note that this frees only the array itself.  The pointed-to
     configuration objects should have been deallocated by res_nclose
     and per-thread cleanup functions.  */
  resolv_conf_array_free (&global->array);

  free (global);

  /* Stop potential future __resolv_conf_detach calls from accessing
     deallocated memory.  */
  global = NULL;
}
text_set_element (__libc_subfreeres, freeres);
