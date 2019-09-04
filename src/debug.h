#ifndef LSM9DS1_DEBUG_H_
#define LSM9DS1_DEBUG_H_

#if DEBUG > 0
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, "[%s:%d:%s()]: " fmt, \
		__FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) /* Don't do anything in release builds */
#endif

#endif