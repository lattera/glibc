/* Test module for bug-dlsym1.c test case.  */

extern int dlopen_test_variable;

/* here to get the unresolved symbol in our .so */
char foo()
{
    return dlopen_test_variable;
}
