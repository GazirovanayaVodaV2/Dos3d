#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define Bit(val) (val * 8)
#define KiB(val) (val * 1024)
#define MiB(val) KiB(val * 1024)

typedef uint8_t byte;
typedef uint8_t u8;
typedef int8_t i8;

typedef uint16_t word;
typedef uint16_t u16;
typedef int16_t i16;

typedef uint32_t u32;
typedef int32_t i32;

typedef uint64_t u64;
typedef int64_t i64;

typedef float number;

#endif