#ifndef _TYPES_H_
#define _TYPES_H_

typedef	signed char	int8_t;
typedef	unsigned char uint8_t;
typedef	short int16_t;
typedef	unsigned short uint16_t;
typedef	int int32_t;
typedef	unsigned int uint32_t;
#if __SIZEOF_LONG__ == 8
typedef	long int64_t;
typedef	unsigned long uint64_t;
#elif __SIZEOF_LONG__ == 4
__extension__
typedef	long long int64_t;
__extension__
typedef	unsigned long long uint64_t;
#else
#error "Unsupported long size"
#endif

#if __SIZEOF_SIZE_T__ == 8
typedef	uint64_t size_t;
typedef	int64_t ssize_t;	/* Byte count or error */
#elif __SIZEOF_SIZE_T__ == 4
typedef	uint32_t size_t;
typedef	int32_t ssize_t;	/* Byte count or error */
#else
#error "Unsupported size_t size"
#endif

typedef size_t uintptr_t;
typedef size_t ptrdiff_t;
typedef size_t off_t;

#endif
