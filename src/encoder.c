#include <columns.h>
#include <msgpack.h>

#include "internal.h"

struct Encoder
{
    bool hasError;

    const uint8_t *addr;
    size_t size;
    ptrdiff_t offset;

    size_t capacity;
    size_t wpos;
    uint8_t *buf;

    msgpack_packer packer;
};

static inline int WriteBuf(void* context, const char* buf, size_t len)
{
    struct Encoder *encoder = (struct Encoder *) context;

    if (encoder->wpos + len > encoder->capacity)
    {
        return -1;
    }

    memcpy(
        encoder->buf + encoder->wpos,
        buf,
        len
    );

    encoder->wpos += len;
    return 0;
}

static inline void VisitNumber(
    const clHandler *handler, 
    uint8_t kind,
    struct Encoder *encoder)
{
    struct StorageUnion storage;
    const ptrdiff_t offset = encoder->offset;

    CHECK_MEMORY(
        encoder,
        kind);

    switch (kind)
    {
        case cl_COLUMN_INT8:
            msgpack_pack_int8(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, int8_t, storage));
            break ;
        case cl_COLUMN_INT16:
            msgpack_pack_int16(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, int16_t, storage));
            break ;
        case cl_COLUMN_INT32:
            msgpack_pack_int32(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, int32_t, storage));
            break ;
        case cl_COLUMN_INT64:
            msgpack_pack_int64(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, int64_t, storage));
            break ;
        case cl_COLUMN_UINT8:
            msgpack_pack_uint8(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, uint8_t, storage));
            break ;
        case cl_COLUMN_UINT16:
            msgpack_pack_uint16(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, uint16_t, storage));
            break ;
        case cl_COLUMN_UINT32:
            msgpack_pack_uint32(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, uint32_t, storage));
            break ;
        case cl_COLUMN_UINT64:
            msgpack_pack_uint64(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, uint64_t, storage));
            break ;
        case cl_COLUMN_FLOAT32:
            msgpack_pack_float(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, float, storage));
            break ;
        case cl_COLUMN_FLOAT64:
            msgpack_pack_double(
                &encoder->packer,
                UNSAFE_READ_MEMORY(encoder->addr + offset, double, storage));
            break ;
        case cl_COLUMN_BOOL:
            UNSAFE_READ_MEMORY(encoder->addr + offset, bool, storage) ?
                msgpack_pack_true(
                    &encoder->packer) :
                msgpack_pack_false(
                    &encoder->packer);
            break ;
        default:
            assert(false);
            break ;
    }
}

static inline void VisitArray(
    const clHandler *handler, 
    uint32_t num,
    uint8_t kind,
    const clColumn *element,
    struct Encoder *encoder)
{
    const ptrdiff_t offset = encoder->offset;

    CHECK_COND_ERROR(
        encoder,
        msgpack_pack_array(
            &encoder->packer,
            num) >= 0);

    uint32_t i = 0;

    if (element != NULL)
    {
        const uint32_t stride = element->size;

        for (i = 0; i < num; ++ i)
        {
            encoder->offset = offset + stride * i;

            clVisitChildren(
                handler, 
                element, 
                encoder);

            CHECK_CTX_ERROR(encoder);
        }
    }
    else
    {
        for (i = 0; i < num; ++ i)
        {
            encoder->offset = offset + SIZE(kind) * i;

            VisitNumber(
                handler, 
                kind,
                encoder);

            CHECK_CTX_ERROR(encoder);
        }
    }
}

static void VisitNumberHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Encoder *encoder)
{
    VisitNumber(
        handler,
        column->kind,
        encoder);
}

static inline void VisitObjectHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Encoder *encoder)
{
    const ptrdiff_t offset = encoder->offset;

    const uint32_t num = column->viaObject.num;
    const clColumn *columns = column->viaObject.fields;

    CHECK_COND_ERROR(
        encoder,
        msgpack_pack_array(
            &encoder->packer,
            num) >= 0);

    uint32_t i = 0;

    for (i = 0; i < num; ++ i)
    {
        encoder->offset = offset + columns[i].offset;

        clVisitChildren(
            handler, 
            columns + i,
            encoder);

        CHECK_CTX_ERROR(encoder);
    }
}

static inline void VisitUnionHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Encoder *encoder)
{
    const ptrdiff_t offset = encoder->offset;

    const clColumn *prev_column = column - 1;

    const uint32_t pos = ReadMemoryAsUInt32(
        prev_column->kind, 
        encoder->addr + offset + prev_column->offset - column->offset);

    CHECK_COND_ERROR(
        encoder,
        pos && pos <= column->viaObject.num);

    clVisitChildren(
        handler, 
        column->viaUnion.fields + pos - 1,
        encoder);
}

static inline void VisitFixedArrayHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Encoder *encoder)
{
    VisitArray(
        handler,
        column->viaFixedArray.capacity,
        column->viaFixedArray.flags,
        column->viaFixedArray.element,
        encoder);
}

static inline void VisitFlexibleArrayHandler(
    const clHandler *handler, 
    const clColumn *column, 
    struct Encoder *encoder)
{
    const ptrdiff_t offset = encoder->offset;

    const clColumn *prev_column = column - 1;

    const uint32_t num = ReadMemoryAsUInt32(
        prev_column->kind, 
        encoder->addr + offset + prev_column->offset - column->offset);

    VisitArray(
        handler,
        num,
        column->viaFlexibleArray.flags,
        column->viaFlexibleArray.element,
        encoder);
}

static struct clHandler handler = {
    .visitNumber = (clVisitHandler) VisitNumberHandler,
    .visitObject = (clVisitHandler) VisitObjectHandler,
    .visitUnion = (clVisitHandler) VisitUnionHandler,
    .visitFixedArray = (clVisitHandler) VisitFixedArrayHandler,
    .visitFlexibleArray = (clVisitHandler) VisitFlexibleArrayHandler,
};

size_t cToBuf(
    const clColumn *column, 
    const void *addr, 
    size_t size,
    uint8_t *buf, 
    size_t len)
{
    struct Encoder encoder = {
        .hasError = false,
        .addr = (const uint8_t *) addr,
        .size = size,
        .offset = 0,
        .capacity = len,
        .wpos = 0,
        .buf = buf
    };

    msgpack_packer_init(
        &encoder.packer, 
        &encoder, 
        WriteBuf);

    clVisitChildren(
        &handler,
        column,
        &encoder);

    return !encoder.hasError ?
        encoder.wpos :
        0;
}
