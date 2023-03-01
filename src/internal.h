#pragma once

#include <inttypes.h>

#define DEREF(ptr, type) (*((type *) (ptr)))

#define CHECK_COND_ERROR(ctx, cond) \
    do { \
        if (!(cond)) { \
            ctx->hasError = true; \
            return ; \
        } \
    } while(false)

#define CHECK_CTX_ERROR(ctx) \
    do { \
        if (ctx->hasError) { \
            return ; \
        } \
    } while(false)

#define SIZE(kind) \
    (1ul << (((kind % 6) ? (kind % 6) : 6) - 1))

#define CHECK_MEMORY(ctx, kind) \
    do { \
        if (ctx->offset + SIZE(kind) > ctx->size) { \
            ctx->hasError = true; \
            return ; \
        } \
    } while(false)

uint32_t ReadMemoryAsUInt32(uint8_t kind, const void *addr);
