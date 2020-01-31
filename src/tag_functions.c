#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tagstruct.h"
#include "hresult.h"



/*
//Only used for speedway communication via ethernet
int getTagLength(char* search, int lengthToCheck)
{
    int retVal = 0;

    while (retVal < lengthToCheck)
    {
        if (*search == '\n')
        {
            return retVal;
        }

        if (*search == '\r')
        {
            //found CR, is it followed by LF?
            if (*(search + 1) == '\n')
            {
                return retVal;
            }
        }
        else
        {
            ++search;
            ++retVal;
        }
    }
    return retVal;
}
*/
