#ifndef INT_TYPES_H
#define INT_TYPES_H

typedef char               int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned long long size_t;
#define NULL ((void *)0)

#define UINT64_MAX ((uint64_t)-1)
#define UINT32_MAX ((uint32_t)-1)
#define UINT16_MAX ((uint16_t)-1)
#define UINT8_MAX  ((uint8_t)-1)
#define INT64_MAX  ((int64_t)(UINT64_MAX >> 1))
#define INT32_MAX  ((int32_t)(UINT32_MAX >> 1))
#define INT16_MAX  ((int16_t)(UINT16_MAX >> 1))
#define INT8_MAX   ((int8_t)(UINT8_MAX >> 1))
#define INT64_MIN  ((int64_t)(-INT64_MAX - 1))
#define INT32_MIN  ((int32_t)(-INT32_MAX - 1))
#define INT16_MIN  ((int16_t)(-INT16_MAX - 1))
#define INT8_MIN   ((int8_t)(-INT8_MAX - 1))

#endif // INT_TYPES_H