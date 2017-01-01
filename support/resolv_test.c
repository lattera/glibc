/* DNS test framework and libresolv redirection.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
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

#include <support/resolv_test.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <nss.h>
#include <resolv.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>
#include <support/test-driver.h>
#include <support/xsocket.h>
#include <support/xthread.h>
#include <unistd.h>

/* Response builder. */

enum
  {
    max_response_length = 65536
  };

/* List of pointers to be freed.  The hash table implementation
   (struct hsearch_data) does not provide a way to deallocate all
   objects, so this approach is used to avoid memory leaks.  */
struct to_be_freed
{
  struct to_be_freed *next;
  void *ptr;
};

struct resolv_response_builder
{
  const unsigned char *query_buffer;
  size_t query_length;

  size_t offset;                /* Bytes written so far in buffer.  */
  ns_sect section;              /* Current section in the DNS packet.  */
  unsigned int truncate_bytes;  /* Bytes to remove at end of response. */
  bool drop;                    /* Discard generated response.  */
  bool close;                   /* Close TCP client connection.  */

  /* Offset of the two-byte RDATA length field in the currently
     written RDATA sub-structure.  0 if no RDATA is being written.  */
  size_t current_rdata_offset;

  /* Hash table for locating targets for label compression.  */
  struct hsearch_data compression_offsets;
  /* List of pointers which need to be freed.  Used for domain names
     involved in label compression.  */
  struct to_be_freed *to_be_freed;

  /* Must be last.  Not zeroed for performance reasons.  */
  unsigned char buffer[max_response_length];
};

/* Response builder. */

/* Add a pointer to the list of pointers to be freed when B is
   deallocated.  */
static void
response_push_pointer_to_free (struct resolv_response_builder *b, void *ptr)
{
  if (ptr == NULL)
    return;
  struct to_be_freed *e = xmalloc (sizeof (*e));
  *e = (struct to_be_freed) {b->to_be_freed, ptr};
  b->to_be_freed = e;
}

void
resolv_response_init (struct resolv_response_builder *b,
                      struct resolv_response_flags flags)
{
  if (b->offset > 0)
    FAIL_EXIT1 ("response_init: called at offset %zu", b->offset);
  if (b->query_length < 12)
    FAIL_EXIT1 ("response_init called for a query of size %zu",
                b->query_length);
  if (flags.rcode > 15)
    FAIL_EXIT1 ("response_init: invalid RCODE %u", flags.rcode);

  /* Copy the transaction ID.  */
  b->buffer[0] = b->query_buffer[0];
  b->buffer[1] = b->query_buffer[1];

  /* Initialize the flags.  */
  b->buffer[2] = 0x80;                       /* Mark as response.   */
  b->buffer[2] |= b->query_buffer[2] & 0x01; /* Copy the RD bit.  */
  if (flags.tc)
    b->buffer[2] |= 0x02;
  b->buffer[3] = 0x80 | flags.rcode; /* Always set RA.  */

  /* Fill in the initial section count values.  */
  b->buffer[4] = flags.qdcount >> 8;
  b->buffer[5] = flags.qdcount;
  b->buffer[6] = flags.ancount >> 8;
  b->buffer[7] = flags.ancount;
  b->buffer[8] = flags.nscount >> 8;
  b->buffer[9] = flags.nscount;
  b->buffer[10] = flags.adcount >> 8;
  b->buffer[11] = flags.adcount;

  b->offset = 12;
}

void
resolv_response_section (struct resolv_response_builder *b, ns_sect section)
{
  if (b->offset == 0)
    FAIL_EXIT1 ("resolv_response_section: response_init not called before");
  if (section < b->section)
    FAIL_EXIT1 ("resolv_response_section: cannot go back to previous section");
  b->section = section;
}

/* Add a single byte to B.  */
static inline void
response_add_byte (struct resolv_response_builder *b, unsigned char ch)
{
  if (b->offset == max_response_length)
    FAIL_EXIT1 ("DNS response exceeds 64 KiB limit");
  b->buffer[b->offset] = ch;
  ++b->offset;
}

/* Add a 16-bit word VAL to B, in big-endian format.  */
static void
response_add_16 (struct resolv_response_builder *b, uint16_t val)
{
  response_add_byte (b, val >> 8);
  response_add_byte (b, val);
}

/* Increment the pers-section record counter in the packet header.  */
static void
response_count_increment (struct resolv_response_builder *b)
{
  unsigned int offset = b->section;
  offset = 4 + 2 * offset;
  ++b->buffer[offset + 1];
  if (b->buffer[offset + 1] == 0)
    {
      /* Carry.  */
      ++b->buffer[offset];
      if (b->buffer[offset] == 0)
        /* Overflow.  */
        FAIL_EXIT1 ("too many records in section");
    }
}

void
resolv_response_add_question (struct resolv_response_builder *b,
                              const char *name, uint16_t class, uint16_t type)
{
  if (b->offset == 0)
    FAIL_EXIT1 ("resolv_response_add_question: "
                "resolv_response_init not called");
  if (b->section != ns_s_qd)
    FAIL_EXIT1 ("resolv_response_add_question: "
                "must be called in the question section");

  resolv_response_add_name (b, name);
  response_add_16 (b, type);
  response_add_16 (b, class);

  response_count_increment (b);
}

void
resolv_response_add_name (struct resolv_response_builder *b,
                          const char *const origname)
{
  /* Normalized name.  */
  char *name;
  /* Normalized name with case preserved.  */
  char *name_case;
  {
    size_t namelen = strlen (origname);
    /* Remove trailing dots.  FIXME: Handle trailing quoted dots.  */
    while (namelen > 0 && origname[namelen - 1] == '.')
      --namelen;
    name = xmalloc (namelen + 1);
    name_case = xmalloc (namelen + 1);
    /* Copy and convert to lowercase.  FIXME: This needs to normalize
       escaping as well.  */
    for (size_t i = 0; i < namelen; ++i)
      {
        char ch = origname[i];
        name_case[i] = ch;
        if ('A' <= ch && ch <= 'Z')
          ch = ch - 'A' + 'a';
        name[i] = ch;
      }
    name[namelen] = 0;
    name_case[namelen] = 0;
  }
  char *name_start = name;
  char *name_case_start = name_case;

  bool compression = false;
  while (*name)
    {
      /* Search for a previous name we can reference.  */
      ENTRY new_entry =
        {
          .key = name,
          .data = (void *) (uintptr_t) b->offset,
        };

      /* If the label can be a compression target because it is at a
         reachable offset, add it to the hash table.  */
      ACTION action;
      if (b->offset < (1 << 12))
        action = ENTER;
      else
        action = FIND;

      /* Search for known compression offsets in the hash table.  */
      ENTRY *e;
      if (hsearch_r (new_entry, action, &e, &b->compression_offsets) == 0)
        {
          if (action == FIND && errno == ESRCH)
            /* Fall through.  */
            e = NULL;
          else
            FAIL_EXIT1 ("hsearch_r failure in name compression: %m");
        }

      /* The name is known.  Reference the previous location.  */
      if (e != NULL && e->data != new_entry.data)
        {
          size_t old_offset = (uintptr_t) e->data;
          response_add_byte (b, 0xC0 | (old_offset >> 8));
          response_add_byte (b, old_offset);
          compression = true;
          break;
        }

      /* The name does not exist yet.  Write one label.  First, add
         room for the label length.  */
      size_t buffer_label_offset = b->offset;
      response_add_byte (b, 0);

      /* Copy the label.  */
      while (true)
        {
          char ch = *name_case;
          if (ch == '\0')
            break;
          ++name;
          ++name_case;
          if (ch == '.')
            break;
          /* FIXME: Handle escaping.  */
          response_add_byte (b, ch);
        }

      /* Patch in the label length.  */
      size_t label_length = b->offset - buffer_label_offset - 1;
      if (label_length == 0)
        FAIL_EXIT1 ("empty label in name compression: %s", origname);
      if (label_length > 63)
        FAIL_EXIT1 ("label too long in name compression: %s", origname);
      b->buffer[buffer_label_offset] = label_length;

      /* Continue with the tail of the name and the next label.  */
    }

  if (compression)
    {
      /* If we found an immediate match for the name, we have not put
         it into the hash table, and can free it immediately.  */
      if (name == name_start)
        free (name_start);
      else
        response_push_pointer_to_free (b, name_start);
    }
  else
    {
      /* Terminate the sequence of labels.  With compression, this is
         implicit in the compression reference.  */
      response_add_byte (b, 0);
      response_push_pointer_to_free (b, name_start);
    }

  free (name_case_start);
}

void
resolv_response_open_record (struct resolv_response_builder *b,
                             const char *name,
                             uint16_t class, uint16_t type, uint32_t ttl)
{
  if (b->section == ns_s_qd)
    FAIL_EXIT1 ("resolv_response_open_record called in question section");
  if (b->current_rdata_offset != 0)
    FAIL_EXIT1 ("resolv_response_open_record called with open record");

  resolv_response_add_name (b, name);
  response_add_16 (b, type);
  response_add_16 (b, class);
  response_add_16 (b, ttl >> 16);
  response_add_16 (b, ttl);

  b->current_rdata_offset = b->offset;
  /* Add room for the RDATA length.  */
  response_add_16 (b, 0);
}


void
resolv_response_close_record (struct resolv_response_builder *b)
{
  size_t rdata_offset = b->current_rdata_offset;
  if (rdata_offset == 0)
    FAIL_EXIT1 ("response_close_record called without open record");
  size_t rdata_length = b->offset - rdata_offset - 2;
  if (rdata_length > 65535)
    FAIL_EXIT1 ("RDATA length %zu exceeds limit", rdata_length);
  b->buffer[rdata_offset] = rdata_length >> 8;
  b->buffer[rdata_offset + 1] = rdata_length;
  response_count_increment (b);
  b->current_rdata_offset = 0;
}

void
resolv_response_add_data (struct resolv_response_builder *b,
                          const void *data, size_t length)
{
  size_t remaining = max_response_length - b->offset;
  if (remaining < length)
    FAIL_EXIT1 ("resolv_response_add_data: not enough room for %zu bytes",
                length);
  memcpy (b->buffer + b->offset, data, length);
  b->offset += length;
}

void
resolv_response_drop (struct resolv_response_builder *b)
{
  b->drop = true;
}

void
resolv_response_close (struct resolv_response_builder *b)
{
  b->close = true;
}

void
resolv_response_truncate_data (struct resolv_response_builder *b, size_t count)
{
  if (count > 65535)
    FAIL_EXIT1 ("resolv_response_truncate_data: argument too large: %zu",
                count);
  b->truncate_bytes = count;
}


size_t
resolv_response_length (const struct resolv_response_builder *b)
{
  return b->offset;
}

unsigned char *
resolv_response_buffer (const struct resolv_response_builder *b)
{
  unsigned char *result = xmalloc (b->offset);
  memcpy (result, b->buffer, b->offset);
  return result;
}

static struct resolv_response_builder *
response_builder_allocate
  (const unsigned char *query_buffer, size_t query_length)
{
  struct resolv_response_builder *b = xmalloc (sizeof (*b));
  memset (b, 0, offsetof (struct resolv_response_builder, buffer));
  b->query_buffer = query_buffer;
  b->query_length = query_length;
  TEST_VERIFY_EXIT (hcreate_r (10000, &b->compression_offsets) != 0);
  return b;
}

static void
response_builder_free (struct resolv_response_builder *b)
{
  struct to_be_freed *current = b->to_be_freed;
  while (current != NULL)
    {
      struct to_be_freed *next = current->next;
      free (current->ptr);
      free (current);
      current = next;
    }
  hdestroy_r (&b->compression_offsets);
  free (b);
}

/* DNS query processing. */

/* Data extracted from the question section of a DNS packet.  */
struct query_info
{
  char qname[MAXDNAME];
  uint16_t qclass;
  uint16_t qtype;
};

/* Update *INFO from the specified DNS packet.  */
static void
parse_query (struct query_info *info,
             const unsigned char *buffer, size_t length)
{
  if (length < 12)
    FAIL_EXIT1 ("malformed DNS query: too short: %zu bytes", length);

  int ret = dn_expand (buffer, buffer + length, buffer + 12,
                       info->qname, sizeof (info->qname));
  if (ret < 0)
    FAIL_EXIT1 ("malformed DNS query: cannot uncompress QNAME");

  /* Obtain QTYPE and QCLASS.  */
  size_t remaining = length - (12 + ret);
  struct
  {
    uint16_t qtype;
    uint16_t qclass;
  } qtype_qclass;
  if (remaining < sizeof (qtype_qclass))
    FAIL_EXIT1 ("malformed DNS query: "
                "query lacks QCLASS/QTYPE, QNAME: %s", info->qname);
  memcpy (&qtype_qclass, buffer + 12 + ret, sizeof (qtype_qclass));
  info->qclass = ntohs (qtype_qclass.qclass);
  info->qtype = ntohs (qtype_qclass.qtype);
}


/* Main testing framework.  */

/* Per-server information.  One struct is allocated for each test
   server.  */
struct resolv_test_server
{
  /* Local address of the server.  UDP and TCP use the same port.  */
  struct sockaddr_in address;

  /* File descriptor of the UDP server, or -1 if this server is
     disabled.  */
  int socket_udp;

  /* File descriptor of the TCP server, or -1 if this server is
     disabled.  */
  int socket_tcp;

  /* Counter of the number of responses processed so far.  */
  size_t response_number;

  /* Thread handles for the server threads (if not disabled in the
     configuration).  */
  pthread_t thread_udp;
  pthread_t thread_tcp;
};

/* Main struct for keeping track of libresolv redirection and
   testing.  */
struct resolv_test
{
  /* After initialization, any access to the struct must be performed
     while this lock is acquired.  */
  pthread_mutex_t lock;

  /* Data for each test server. */
  struct resolv_test_server servers[resolv_max_test_servers];

  /* Used if config.single_thread_udp is true.  */
  pthread_t thread_udp_single;

  struct resolv_redirect_config config;
  bool termination_requested;
};

/* Function implementing a server thread.  */
typedef void (*thread_callback) (struct resolv_test *, int server_index);

/* Storage for thread-specific data, for passing to the
   thread_callback function.  */
struct thread_closure
{
  struct resolv_test *obj;      /* Current test object.  */
  thread_callback callback;     /* Function to call.  */
  int server_index;             /* Index of the implemented server.  */
};

/* Wrap response_callback as a function which can be passed to
   pthread_create.  */
static void *
thread_callback_wrapper (void *arg)
{
  struct thread_closure *closure = arg;
  closure->callback (closure->obj, closure->server_index);
  free (closure);
  return NULL;
}

/* Start a server thread for the specified SERVER_INDEX, implemented
   by CALLBACK.  */
static pthread_t
start_server_thread (struct resolv_test *obj, int server_index,
                     thread_callback callback)
{
  struct thread_closure *closure = xmalloc (sizeof (*closure));
  *closure = (struct thread_closure)
    {
      .obj = obj,
      .callback = callback,
      .server_index = server_index,
    };
  return xpthread_create (NULL, thread_callback_wrapper, closure);
}

/* Process one UDP query.  Return false if a termination requested has
   been detected.  */
static bool
server_thread_udp_process_one (struct resolv_test *obj, int server_index)
{
  unsigned char query[512];
  struct sockaddr_storage peer;
  socklen_t peerlen = sizeof (peer);
  size_t length = xrecvfrom (obj->servers[server_index].socket_udp,
                             query, sizeof (query), 0,
                             (struct sockaddr *) &peer, &peerlen);
  /* Check for termination.  */
  {
    bool termination_requested;
    xpthread_mutex_lock (&obj->lock);
    termination_requested = obj->termination_requested;
    xpthread_mutex_unlock (&obj->lock);
    if (termination_requested)
      return false;
  }


  struct query_info qinfo;
  parse_query (&qinfo, query, length);
  if (test_verbose > 0)
    {
      if (test_verbose > 1)
        printf ("info: UDP server %d: incoming query:"
                " %zd bytes, %s/%u/%u, tnxid=0x%02x%02x\n",
                server_index, length, qinfo.qname, qinfo.qclass, qinfo.qtype,
                query[0], query[1]);
      else
        printf ("info: UDP server %d: incoming query:"
                " %zd bytes, %s/%u/%u\n",
                server_index, length, qinfo.qname, qinfo.qclass, qinfo.qtype);
    }

  struct resolv_response_context ctx =
    {
      .query_buffer = query,
      .query_length = length,
      .server_index = server_index,
      .tcp = false,
    };
  struct resolv_response_builder *b = response_builder_allocate (query, length);
  obj->config.response_callback
    (&ctx, b, qinfo.qname, qinfo.qclass, qinfo.qtype);

  if (b->drop)
    {
      if (test_verbose)
        printf ("info: UDP server %d: dropping response to %s/%u/%u\n",
                server_index, qinfo.qname, qinfo.qclass, qinfo.qtype);
    }
  else
    {
      if (test_verbose)
        {
          if (b->offset >= 12)
            printf ("info: UDP server %d: sending response:"
                    " %zu bytes, RCODE %d (for %s/%u/%u)\n",
                    server_index, b->offset, b->buffer[3] & 0x0f,
                    qinfo.qname, qinfo.qclass, qinfo.qtype);
          else
            printf ("info: UDP server %d: sending response: %zu bytes"
                    " (for %s/%u/%u)\n",
                    server_index, b->offset,
                    qinfo.qname, qinfo.qclass, qinfo.qtype);
          if (b->truncate_bytes > 0)
            printf ("info:    truncated by %u bytes\n", b->truncate_bytes);
        }
      size_t to_send = b->offset;
      if (to_send < b->truncate_bytes)
        to_send = 0;
      else
        to_send -= b->truncate_bytes;

      /* Ignore most errors here because the other end may have closed
         the socket. */
      if (sendto (obj->servers[server_index].socket_udp,
                  b->buffer, to_send, 0,
                  (struct sockaddr *) &peer, peerlen) < 0)
        TEST_VERIFY_EXIT (errno != EBADF);
    }
  response_builder_free (b);
  return true;
}

/* UDP thread_callback function.  Variant for one thread per
   server.  */
static void
server_thread_udp (struct resolv_test *obj, int server_index)
{
  while (server_thread_udp_process_one (obj, server_index))
    ;
}

/* Single-threaded UDP processing function, for the single_thread_udp
   case.  */
static void *
server_thread_udp_single (void *closure)
{
  struct resolv_test *obj = closure;

  struct pollfd fds[resolv_max_test_servers];
  for (int server_index = 0; server_index < resolv_max_test_servers;
       ++server_index)
    if (obj->config.servers[server_index].disable_udp)
      fds[server_index] = (struct pollfd) {.fd = -1};
    else
      {
        fds[server_index] = (struct pollfd)
          {
            .fd = obj->servers[server_index].socket_udp,
            .events = POLLIN
          };

        /* Make the socket non-blocking.  */
        int flags = fcntl (obj->servers[server_index].socket_udp, F_GETFL, 0);
        if (flags < 0)
          FAIL_EXIT1 ("fcntl (F_GETFL): %m");
        flags |= O_NONBLOCK;
        if (fcntl (obj->servers[server_index].socket_udp, F_SETFL, flags) < 0)
          FAIL_EXIT1 ("fcntl (F_SETFL): %m");
      }

  while (true)
    {
      xpoll (fds, resolv_max_test_servers, -1);
      for (int server_index = 0; server_index < resolv_max_test_servers;
           ++server_index)
        if (fds[server_index].revents != 0)
          {
            if (!server_thread_udp_process_one (obj, server_index))
              goto out;
            fds[server_index].revents = 0;
          }
    }

 out:
  return NULL;
}

/* Start the single UDP handler thread (for the single_thread_udp
   case).  */
static void
start_server_thread_udp_single (struct resolv_test *obj)
{
  obj->thread_udp_single
    = xpthread_create (NULL, server_thread_udp_single, obj);
}

/* Data describing a TCP client connect.  */
struct tcp_thread_closure
{
  struct resolv_test *obj;
  int server_index;
  int client_socket;
};

/* Read a complete DNS query packet.  If EOF_OK, an immediate
   end-of-file condition is acceptable.  */
static bool
read_fully (int fd, void *buf, size_t len, bool eof_ok)
{
  const void *const end = buf + len;
  while (buf < end)
    {
      ssize_t ret = read (fd, buf, end - buf);
      if (ret == 0)
        {
          if (!eof_ok)
            {
              support_record_failure ();
              printf ("error: unexpected EOF on TCP connection\n");
            }
          return false;
        }
      else if (ret < 0)
        {
          if (!eof_ok || errno != ECONNRESET)
            {
              support_record_failure ();
              printf ("error: TCP read: %m\n");
            }
          return false;
        }
      buf += ret;
      eof_ok = false;
    }
  return true;
}

/* Write an array of iovecs.  Terminate the process on failure.  */
static void
writev_fully (int fd, struct iovec *buffers, size_t count)
{
  while (count > 0)
    {
      /* Skip zero-length write requests.  */
      if (buffers->iov_len == 0)
        {
          ++buffers;
          --count;
          continue;
        }
      /* Try to rewrite the remaing buffers.  */
      ssize_t ret = writev (fd, buffers, count);
      if (ret < 0)
        FAIL_EXIT1 ("writev: %m");
      if (ret == 0)
        FAIL_EXIT1 ("writev: invalid return value zero");
      /* Find the buffers that were successfully written.  */
      while (ret > 0)
        {
          if (count == 0)
            FAIL_EXIT1 ("internal writev consistency failure");
          /* Current buffer was partially written.  */
          if (buffers->iov_len > (size_t) ret)
            {
              buffers->iov_base += ret;
              buffers->iov_len -= ret;
              ret = 0;
            }
          else
            {
              ret -= buffers->iov_len;
              buffers->iov_len = 0;
              ++buffers;
              --count;
            }
        }
    }
}

/* Thread callback for handling a single established TCP connection to
   a client.  */
static void *
server_thread_tcp_client (void *arg)
{
  struct tcp_thread_closure *closure = arg;

  while (true)
    {
      /* Read packet length.  */
      uint16_t query_length;
      if (!read_fully (closure->client_socket,
                       &query_length, sizeof (query_length), true))
        break;
      query_length = ntohs (query_length);

      /* Read the packet.  */
      unsigned char *query_buffer = xmalloc (query_length);
      read_fully (closure->client_socket, query_buffer, query_length, false);

      struct query_info qinfo;
      parse_query (&qinfo, query_buffer, query_length);
      if (test_verbose > 0)
        {
          if (test_verbose > 1)
            printf ("info: UDP server %d: incoming query:"
                    " %d bytes, %s/%u/%u, tnxid=0x%02x%02x\n",
                    closure->server_index, query_length,
                    qinfo.qname, qinfo.qclass, qinfo.qtype,
                    query_buffer[0], query_buffer[1]);
          else
            printf ("info: TCP server %d: incoming query:"
                    " %u bytes, %s/%u/%u\n",
                    closure->server_index, query_length,
                    qinfo.qname, qinfo.qclass, qinfo.qtype);
        }

      struct resolv_response_context ctx =
        {
          .query_buffer = query_buffer,
          .query_length = query_length,
          .server_index = closure->server_index,
          .tcp = true,
        };
      struct resolv_response_builder *b = response_builder_allocate
        (query_buffer, query_length);
      closure->obj->config.response_callback
        (&ctx, b, qinfo.qname, qinfo.qclass, qinfo.qtype);

      if (b->drop)
        {
          if (test_verbose)
            printf ("info: TCP server %d: dropping response to %s/%u/%u\n",
                    closure->server_index,
                    qinfo.qname, qinfo.qclass, qinfo.qtype);
        }
      else
        {
          if (test_verbose)
            printf ("info: TCP server %d: sending response: %zu bytes"
                    " (for %s/%u/%u)\n",
                    closure->server_index, b->offset,
                    qinfo.qname, qinfo.qclass, qinfo.qtype);
          uint16_t length = htons (b->offset);
          size_t to_send = b->offset;
          if (to_send < b->truncate_bytes)
            to_send = 0;
          else
            to_send -= b->truncate_bytes;
          struct iovec buffers[2] =
            {
              {&length, sizeof (length)},
              {b->buffer, to_send}
            };
          writev_fully (closure->client_socket, buffers, 2);
        }
      bool close_flag = b->close;
      response_builder_free (b);
      free (query_buffer);
      if (close_flag)
        break;
    }

  close (closure->client_socket);
  free (closure);
  return NULL;
}

/* thread_callback for the TCP case.  Accept connections and create a
   new thread for each client.  */
static void
server_thread_tcp (struct resolv_test *obj, int server_index)
{
  while (true)
    {
      /* Get the client conenction.  */
      int client_socket = xaccept
        (obj->servers[server_index].socket_tcp, NULL, NULL);

      /* Check for termination.  */
      xpthread_mutex_lock (&obj->lock);
      if (obj->termination_requested)
        {
          xpthread_mutex_unlock (&obj->lock);
          close (client_socket);
          break;
        }
      xpthread_mutex_unlock (&obj->lock);

      /* Spawn a new thread for handling this connection.  */
      struct tcp_thread_closure *closure = xmalloc (sizeof (*closure));
      *closure = (struct tcp_thread_closure)
        {
          .obj = obj,
          .server_index = server_index,
          .client_socket = client_socket,
        };

      pthread_t thr
        = xpthread_create (NULL, server_thread_tcp_client, closure);
      /* TODO: We should keep track of this thread so that we can
         block in resolv_test_end until it has exited.  */
      xpthread_detach (thr);
    }
}

/* Create UDP and TCP server sockets.  */
static void
make_server_sockets (struct resolv_test_server *server)
{
  while (true)
    {
      server->socket_udp = xsocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      server->socket_tcp = xsocket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

      /* Pick the address for the UDP socket.  */
      server->address = (struct sockaddr_in)
        {
          .sin_family = AF_INET,
          .sin_addr = {.s_addr = htonl (INADDR_LOOPBACK)}
        };
      xbind (server->socket_udp,
             (struct sockaddr *)&server->address, sizeof (server->address));

      /* Retrieve the address. */
      socklen_t addrlen = sizeof (server->address);
      xgetsockname (server->socket_udp,
                    (struct sockaddr *)&server->address, &addrlen);

      /* Bind the TCP socket to the same address.  */
      {
        int on = 1;
        xsetsockopt (server->socket_tcp, SOL_SOCKET, SO_REUSEADDR,
                     &on, sizeof (on));
      }
      if (bind (server->socket_tcp,
                (struct sockaddr *)&server->address,
                sizeof (server->address)) != 0)
        {
          /* Port collision.  The UDP bind succeeded, but the TCP BIND
             failed.  We assume here that the kernel will pick the
             next local UDP address randomly.  */
          if (errno == EADDRINUSE)
            {
              close (server->socket_udp);
              close (server->socket_tcp);
              continue;
            }
          FAIL_EXIT1 ("TCP bind: %m");
        }
      xlisten (server->socket_tcp, 5);
      break;
    }
}

/* One-time initialization of NSS.  */
static void
resolv_redirect_once (void)
{
  /* Only use nss_dns.  */
  __nss_configure_lookup ("hosts", "dns");
  __nss_configure_lookup ("networks", "dns");
  /* Enter a network namespace for isolation and firewall state
     cleanup.  The tests will still work if these steps fail, but they
     may be less reliable.  */
  support_become_root ();
  support_enter_network_namespace ();
}
pthread_once_t resolv_redirect_once_var = PTHREAD_ONCE_INIT;

void
resolv_test_init (void)
{
  /* Perform one-time initialization of NSS.  */
  xpthread_once (&resolv_redirect_once_var, resolv_redirect_once);
}

/* Copy the search path from CONFIG.search to the _res object.  */
static void
set_search_path (struct resolv_redirect_config config)
{
  memset (_res.defdname, 0, sizeof (_res.defdname));
  memset (_res.dnsrch, 0, sizeof (_res.dnsrch));

  char *current = _res.defdname;
  char *end = current + sizeof (_res.defdname);

  for (unsigned int i = 0;
       i < sizeof (config.search) / sizeof (config.search[0]); ++i)
    {
      if (config.search[i] == NULL)
        continue;

      size_t length = strlen (config.search[i]) + 1;
      size_t remaining = end - current;
      TEST_VERIFY_EXIT (length <= remaining);
      memcpy (current, config.search[i], length);
      _res.dnsrch[i] = current;
      current += length;
    }
}

struct resolv_test *
resolv_test_start (struct resolv_redirect_config config)
{
  /* Apply configuration defaults.  */
  if (config.nscount == 0)
    config.nscount = resolv_max_test_servers;

  struct resolv_test *obj = xmalloc (sizeof (*obj));
  *obj = (struct resolv_test) {
    .config = config,
    .lock = PTHREAD_MUTEX_INITIALIZER,
  };

  resolv_test_init ();

  /* Create all the servers, to reserve the necessary ports.  */
  for (int server_index = 0; server_index < config.nscount; ++server_index)
    make_server_sockets (obj->servers + server_index);

  /* Start server threads.  Disable the server ports, as
     requested.  */
  for (int server_index = 0; server_index < config.nscount; ++server_index)
    {
      struct resolv_test_server *server = obj->servers + server_index;
      if (config.servers[server_index].disable_udp)
        {
          close (server->socket_udp);
          server->socket_udp = -1;
        }
      else if (!config.single_thread_udp)
        server->thread_udp = start_server_thread (obj, server_index,
                                                  server_thread_udp);
      if (config.servers[server_index].disable_tcp)
        {
          close (server->socket_tcp);
          server->socket_tcp = -1;
        }
      else
        server->thread_tcp = start_server_thread (obj, server_index,
                                                  server_thread_tcp);
    }
  if (config.single_thread_udp)
    start_server_thread_udp_single (obj);

  int timeout = 1;

  /* Initialize libresolv.  */
  TEST_VERIFY_EXIT (res_init () == 0);

  /* Disable IPv6 name server addresses.  The code below only
     overrides the IPv4 addresses.  */
  __res_iclose (&_res, true);
  _res._u._ext.nscount = 0;

  /* Redirect queries to the server socket.  */
  if (test_verbose)
    {
      printf ("info: old timeout value: %d\n", _res.retrans);
      printf ("info: old retry attempt value: %d\n", _res.retry);
      printf ("info: old _res.options: 0x%lx\n", _res.options);
      printf ("info: old _res.nscount value: %d\n", _res.nscount);
      printf ("info: old _res.ndots value: %d\n", _res.ndots);
    }
  _res.retrans = timeout;
  _res.retry = 4;
  _res.nscount = config.nscount;
  _res.options = RES_INIT | RES_RECURSE | RES_DEFNAMES | RES_DNSRCH;
  _res.ndots = 1;
  if (test_verbose)
    {
      printf ("info: new timeout value: %d\n", _res.retrans);
      printf ("info: new retry attempt value: %d\n", _res.retry);
      printf ("info: new _res.options: 0x%lx\n", _res.options);
      printf ("info: new _res.nscount value: %d\n", _res.nscount);
      printf ("info: new _res.ndots value: %d\n", _res.ndots);
    }
  for (int server_index = 0; server_index < config.nscount; ++server_index)
    {
      _res.nsaddr_list[server_index] = obj->servers[server_index].address;
      if (test_verbose)
        {
          char buf[256];
          TEST_VERIFY_EXIT
            (inet_ntop (AF_INET, &obj->servers[server_index].address.sin_addr,
                        buf, sizeof (buf)) != NULL);
          printf ("info: server %d: %s/%u\n",
                  server_index, buf,
                  htons (obj->servers[server_index].address.sin_port));
        }
    }

  set_search_path (config);

  return obj;
}

void
resolv_test_end (struct resolv_test *obj)
{
  res_close ();

  xpthread_mutex_lock (&obj->lock);
  obj->termination_requested = true;
  xpthread_mutex_unlock (&obj->lock);

  /* Send trigger packets to unblock the server threads.  */
  for (int server_index = 0; server_index < obj->config.nscount;
       ++server_index)
    {
      if (!obj->config.servers[server_index].disable_udp)
        {
          int sock = xsocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
          xsendto (sock, "", 1, 0,
                   (struct sockaddr *) &obj->servers[server_index].address,
                   sizeof (obj->servers[server_index].address));
          close (sock);
        }
      if (!obj->config.servers[server_index].disable_tcp)
        {
          int sock = xsocket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
          xconnect (sock,
                    (struct sockaddr *) &obj->servers[server_index].address,
                    sizeof (obj->servers[server_index].address));
          close (sock);
        }
    }

  if (obj->config.single_thread_udp)
    xpthread_join (obj->thread_udp_single);

  /* Wait for the server threads to terminate.  */
  for (int server_index = 0; server_index < obj->config.nscount;
       ++server_index)
    {
      if (!obj->config.servers[server_index].disable_udp)
        {
          if (!obj->config.single_thread_udp)
            xpthread_join (obj->servers[server_index].thread_udp);
          close (obj->servers[server_index].socket_udp);
        }
      if (!obj->config.servers[server_index].disable_tcp)
        {
          xpthread_join (obj->servers[server_index].thread_tcp);
          close (obj->servers[server_index].socket_tcp);
        }
    }

  free (obj);
}
