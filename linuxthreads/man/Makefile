SOURCES=pthread_atfork.man pthread_attr_init.man pthread_cancel.man \
  pthread_cleanup_push.man pthread_cond_init.man \
  pthread_condattr_init.man pthread_create.man pthread_detach.man \
  pthread_equal.man pthread_exit.man pthread_join.man \
  pthread_key_create.man pthread_mutex_init.man \
  pthread_mutexattr_init.man pthread_once.man pthread_self.man \
  pthread_setschedparam.man pthread_sigmask.man sem_init.man \
  pthread_kill_other_threads_np.man pthread_mutexattr_setkind_np.man

MANPAGES=$(SOURCES:.man=.3thr)

PREPRO=perl troffprepro

MANDIR=/usr/man/man3

all: $(MANPAGES)

.SUFFIXES: .man .3thr

.man.3thr:
	$(PREPRO) $*.man $*.3thr

$(MANPAGES): troffprepro

clean:
	rm -f *.3thr
	rm -f *~

install:
	install *.3thr $(MANDIR)
	@echo "*** Remember to run /usr/sbin/makewhatis `dirname $(MANDIR)` at some point"
