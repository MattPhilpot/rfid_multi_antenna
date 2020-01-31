#ifndef MEMORY_HELPER_MACROS_H
#define MEMORY_HELPER_MACROS_H

#include "memory_helper.h"

#define SAFE_FREE(pointer) safe_memory_free((void **) &(pointer))

#endif // MEMORY_HELPER_MACROS_H
