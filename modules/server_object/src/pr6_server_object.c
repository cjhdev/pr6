/* Copyright (c) 2016 Cameron Harper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * */

#include "debug_interface.h"
#include "server_object.h"
#include <stddef.h>

static const struct list_element *ObjectList;
static uint16_t ObjectListSize;

void PR05_ServerObjectInit(const struct list_element *ascendingObjectList, uint16_t objectListSize)
{
    ASSERT((ascendingObjectList != NULL))
    
#ifndef NDEBUG
    uint16_t i;
    uint8_t j;
    uint8_t k;

    for(i=0U; i < objectListSize; i++){

        if(i > 0U){

            /* .objectID shall be in ascending order (and unique) */
            ASSERT((ascendingObjectList[i-1U].objectID > ascendingObjectList[i].objectID))
        }

        if(ascendingObjectList[i].method != NULL){

            for(j=0U; j < ascendingObjectList[i].methodSize; j++){

                for(k=0U; k < ascendingObjectList[i].methodSize; k++){

                    if(k != j){

                        /* .methodIndex shall be unique per object method list */
                        ASSERT((ascendingObjectList[i].method[j].methodIndex != ascendingObjectList[i].method[k].methodIndex))
                    }
                }

                /* .adapter shall not be NULL */
                ASSERT((ascendingObjectList[i].method[j].adapter != NULL))
            }
        }
    }
#endif

    ObjectList = ascendingObjectList;
    ObjectListSize = objectListSize;
}

enum pr05_adapter_result PR05_ServerObjectInterface(const struct pr05_server *r, struct pr05_server_adapter *arg, enum pr05_result *result)
{
    enum pr05_adapter_result retval = PR05_ADAPTER_SUCCESS;
    const struct list_element *elem = NULL;
    uint8_t i;
    uint8_t j;
    uint8_t k;
    uint16_t lower = 0U;
    uint16_t upper = ObjectListSize;
    uint16_t mid;
    enum loopControl { LOOP, BREAK_LOOP } loopState = LOOP;

    ASSERT((r))
    ASSERT((arg != NULL))
    ASSERT((result != NULL))    
    ASSERT((ObjectList != NULL))

    *result = PR05_OBJECT_UNDEFINED;

    /* binary search for objectID */
    while(lower < upper){
      
        mid = (lower + upper) >> 1U;

        if(ObjectList[mid].objectID > arg->objectID){

            upper = mid;
        }
        else if(ObjectList[mid].objectID < arg->objectID){

            lower = mid + 1U;
        }
        /* a match */
        else{

            elem = &ObjectList[mid];

            *result = PR05_RESULT_METHOD_UNDEFINED;

            /* look the method number... */
            for(i=0; i < elem->methodSize; i++){

                if(elem->method[i].methodIndex == arg->methodIndex){

                    *result = PR05_RESULT_ACCESS_DENIED;

                    /* check the role is authorised for this method number... */
                    for(j=0; j < elem->method[i].roleSize; j++){

                        for(k=0; k < arg->roleSize; k++){

                            if(elem->method[i].role[j] == arg->role[k]){

                                retval = elem->method[i].adapter(r, arg, result);
                                loopState = BREAK_LOOP;
                                break;
                            }                            
                        }

                        if(loopState == BREAK_LOOP){

                            break;
                        }
                    }

                    break;
                }
            }

            break;
        }   
    }

    return retval;
}
