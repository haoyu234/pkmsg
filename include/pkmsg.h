#pragma once

struct clColumn;

#ifdef __cplusplus
#include <cinttypes>
#include <cstddef>
#else
#include <inttypes.h>
#include <stddef.h>
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

PKMSG_EXPORT size_t cToBuf(const struct clColumn *column, const void *addr,
                           size_t size, uint8_t *buf, size_t len);

PKMSG_EXPORT size_t cFromBuf(const struct clColumn *column, void *addr,
                             size_t size, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif
