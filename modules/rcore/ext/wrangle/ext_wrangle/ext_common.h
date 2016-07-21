#ifndef EXT_COMMON_H
#define EXT_COMMON_H

#include "pr6_client.h"
#include "pr6_server.h"
#include "pr6_peer.h"

#include <ruby.h>

struct state_wrapper {

    VALUE self;
    union {
        struct pr6_client client;
        struct pr6_server server;
    } state;
};

VALUE StateWrapperAlloc(VALUE klass);

VALUE SelfFromWrapper(const void *ptr);

#endif

