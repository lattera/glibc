/* `struct iovec' -- Structure describing a section of memory.  */

struct iovec
{
  /* Starting address.  */
  __ptr_t iov_base;
  /* Length in bytes.  */
  size_t iov_len;
};
