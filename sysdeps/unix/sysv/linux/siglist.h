/* This file is included multiple times.  */

/* Standard signals  */
  init_sig (SIGHUP, "HUP", "Hangup")
  init_sig (SIGINT, "INT", "Interrupt")
  init_sig (SIGQUIT, "QUIT", "Quit")
  init_sig (SIGILL, "ILL", "Illegal Instruction")
  init_sig (SIGTRAP, "TRAP", "Trace/breakpoint trap")
  init_sig (SIGABRT, "ABRT", "Aborted")
  init_sig (SIGFPE, "FPE", "Floating point exception")
  init_sig (SIGKILL, "KILL", "Killed")
  init_sig (SIGBUS, "BUS", "Bus error")
  init_sig (SIGSEGV, "SEGV", "Segmentation fault")
  init_sig (SIGPIPE, "PIPE", "Broken pipe")
  init_sig (SIGALRM, "ALRM", "Alarm clock")
  init_sig (SIGTERM, "TERM", "Terminated")
  init_sig (SIGURG, "URG", "Urgent I/O condition")
  init_sig (SIGSTOP, "STOP", "Stopped (signal)")
  init_sig (SIGTSTP, "TSTP", "Stopped")
  init_sig (SIGCONT, "CONT", "Continued")
  init_sig (SIGCHLD, "CHLD", "Child exited")
  init_sig (SIGTTIN, "TTIN", "Stopped (tty input)")
  init_sig (SIGTTOU, "TTOU", "Stopped (tty output)")
  init_sig (SIGIO, "IO", "I/O possible")
  init_sig (SIGXCPU, "XCPU", "CPU time limit exceeded")
  init_sig (SIGXFSZ, "XFSZ", "File size limit exceeded")
  init_sig (SIGVTALRM, "VTALRM", "Virtual timer expired")
  init_sig (SIGPROF, "PROF", "Profiling timer expired")
  init_sig (SIGWINCH, "WINCH", "Window changed")
  init_sig (SIGUSR1, "USR1", "User defined signal 1")
  init_sig (SIGUSR2, "USR2", "User defined signal 2")

/* Variations  */
#ifdef SIGEMT
  init_sig (SIGEMT, "EMT", "EMT trap")
#endif
#ifdef SIGSYS
  init_sig (SIGSYS, "SYS", "Bad system call")
#endif
#ifdef SIGSTKFLT
  init_sig (SIGSTKFLT, "STKFLT", "Stack fault")
#endif
#ifdef SIGINFO
  init_sig (SIGINFO, "INFO", "Information request")
#elif defined(SIGPWR)
  init_sig (SIGPWR, "PWR", "Power failure")
#endif
