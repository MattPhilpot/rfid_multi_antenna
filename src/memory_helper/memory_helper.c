#include <stdlib.h>
#include "memory_helper.h"

void safe_memory_free(void** pntr_addr)
{
    if (pntr_addr != NULL && *pntr_addr != NULL)
    {
        free(*pntr_addr);
        *pntr_addr = NULL;
    }
}
