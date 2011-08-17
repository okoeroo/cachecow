#ifndef CC_TYPES
#define CC_TYPES

#include <event2/event.h>


typedef enum protocol_e {
    IPv6TCP,
    IPv6UDP,
    IPv4TCP,
    IPv4UDP
} protocol_t;


typedef struct settings_s {
    protocol_t  proto;
    int         listen_port;
    int         remote_port;
    char *      remote_host;
} settings_t;

typedef struct info_s {
    settings_t settings;
    struct event_base *base;
} info_t;



#endif /* CC_TYPES */
