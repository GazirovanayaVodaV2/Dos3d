#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define Bit(val) (val * 8)
#define KiB(val) (val * 1024)
#define MiB(val) KiB(val * 1024)

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#define SWAP(buffer, i1, i2)                      \
	do {                                          \
		__typeof__(buffer[i1]) temp = buffer[i1]; \
		buffer[i1] = buffer[i2];                  \
		buffer[i2] = temp;                        \
	} while (0)

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