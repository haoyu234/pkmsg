#pragma once

struct clColumn;

#ifdef __cplusplus
#include <cinttypes>
#include <cstddef>
#else
#include <inttypes.h>
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

size_t cToBuf(const struct clColumn *column, const void *addr, size_t size,
              uint8_t *buf, size_t len);

size_t cFromBuf(const struct clColumn *column, void *addr, size_t size,
                const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif
