#pragma once

#include <string.h>
#include <stdbool.h>
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

struct StorageUnion
{
    union 
    {
        bool b;

        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;

        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;

        float f;
        double d;

        #if __HAVE_FLOAT16
            _Float16 f16;
        #endif
        #if __HAVE_FLOAT32
            _Float32 f32;
        #endif
        #if __HAVE_FLOAT64
            _Float64 f64;
        #endif
        #if __HAVE_FLOAT128
            _Float128 f128;
        #endif
    };
};

#define UNSAFE_READ_MEMORY(ptr, type, storage) \
    DEREF(memcpy(&storage, ptr, sizeof(type)), type)

#define UNSAFE_WRITE_MEMORY(ptr, type, value) \
    do { \
        type storage = value; \
        memcpy(ptr, &storage, sizeof(type)); \
    } while (false)

uint32_t ReadMemoryAsUInt32(uint8_t kind, const void *addr);
