#ifndef PROXY_INCLUDE_TYPES_H_
#define PROXY_INCLUDE_TYPES_H_

#define FALSE	(0)
#define TRUE	(1)

#define DBX() do { fprintf(stderr,"DBX[%s:%d]\n",__FILE__,__LINE__); fflush(stdout); } while(0) /* For debugging */

#endif

