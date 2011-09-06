/* Data structure to contain the action information.  */
struct __spawn_action
{
  enum
  {
    spawn_do_close,
    spawn_do_dup2,
    spawn_do_open
  } tag;

  union
  {
    struct
    {
      int fd;
    } close_action;
    struct
    {
      int fd;
      int newfd;
    } dup2_action;
    struct
    {
      int fd;
      const char *path;
      int oflag;
      mode_t mode;
    } open_action;
  } action;
};

#define SPAWN_XFLAGS_USE_PATH	0x1
#define SPAWN_XFLAGS_TRY_SHELL	0x2

extern int __posix_spawn_file_actions_realloc (posix_spawn_file_actions_t *
					       file_actions);

extern int __spawni (pid_t *pid, const char *path,
		     const posix_spawn_file_actions_t *file_actions,
		     const posix_spawnattr_t *attrp, char *const argv[],
		     char *const envp[], int xflags);
