#ifndef __MAIN_H
#define __MAIN_H


#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf("[DEBUG][%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

#define UNUSED(x) (void)(x)

#endif // !__MAIN_H