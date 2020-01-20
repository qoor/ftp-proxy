#ifndef __STDINC_H__
#define __STDINC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <malloc.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <netinet/ip.h>

#include <linux/tcp.h>

#include "vector.h"
#include "proxy.h"
#include "log.h"
#include "server.h"
#include "option.h"
#include "packet.h"
#include "main.h"
#include "client.h"

#endif
