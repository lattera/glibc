#include <rt/mqueue.h>

#ifndef _ISOMAC
# if IS_IN (librt)
hidden_proto (mq_timedsend)
hidden_proto (mq_timedreceive)
hidden_proto (mq_setattr)
# endif
#endif
