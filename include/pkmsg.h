#pragma once

struct clColumn;

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h> // IWYU pragma: keep
#endif

#if defined(BUILD_PKMSG_DLL)
#if defined(_WIN32)
// building the pkmsg DLL
#define PKMSG_EXPORT __declspec(dllexport)
#elif defined(_WIN32)
// using the pkmsg DLL
#define PKMSG_EXPORT __declspec(dllimport)
#elif defined(__GNUC__)
// building the pkmsg shared library
#define PKMSG_EXPORT __attribute__((visibility("default")))
#endif
#else
// static library
#define PKMSG_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

PKMSG_EXPORT int32_t pk_encode(const struct clColumn *column, const void *addr,
                               int32_t size, uint8_t *buf, int32_t len);

PKMSG_EXPORT int32_t pk_decode(const struct clColumn *column, void *addr,
                               int32_t size, const uint8_t *buf, int32_t len);

PKMSG_EXPORT int32_t pk_encode_cb(
    const struct clColumn *column, const void *addr, int32_t size, void *ctx,
    int32_t (*write_cb)(void *ctx, const void *data, int32_t count));

PKMSG_EXPORT int32_t pk_decode_cb(const struct clColumn *column, void *addr,
                                  int32_t size, void *ctx,
                                  int32_t (*read_cb)(void *ctx, void *data,
                                                     int32_t count));

#ifdef __cplusplus
}
#endif
