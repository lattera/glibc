/* Stub version of header for fork handling.  Mainly to handle pthread_atfork
   and friends.  Outside dependencies:

   UNREGISTER_ATFORK
     If defined it must expand to a function call which takes one void*
     parameter which is the DSO handle for the DSO which gets unloaded.
     The function so called has to remove the atfork handlers registered
     by this module.  */
