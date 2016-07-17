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
 * 
 * */ 
#ifndef PR6_SERVER_OBJECT_H
#define PR6_SERVER_OBJECT_H

/**
 *
 * @defgroup pr05_server_object PR05 Server Object Interface Implementation
 * 
 * A Server Object Interface useful for embedded systems.
 *
 * @{
 * */

/* includes ***********************************************************/

#include "pr6_server_object_interface.h"


/* structs ************************************************************/

/** Characteristics of a method */
struct method_element {

    uint8_t methodIndex;           /**< method index for this method */
    uint8_t roleSize;           /**< number of role descriptors in list */
    const uint8_t *role;        /**< list of role descriptors authorised to invoke this adapter */   
    pr05_server_object_interface_fn_t adapter;  /**< method adapter function (i.e. the internal method called) */
};

/** Characteristics of an object */
struct list_element {

    uint16_t objectID;                      /**< object identifier */
    uint16_t classID;                       /**< object class identifier */
    uint8_t classVersion;                   /**< object class version */
    uint8_t methodSize;                     /**< number of methods in list */
    const struct method_element *method;    /**< pointer to method list */
};

/* functions **********************************************************/

/**
 * Call once to initialise an object list during node initialisation
 *
 * The original orderedObjectList data structure must be maintained
 * as this function caches a reference to it. This function shall
 * not copy orderedObjectList.
 *
 * The data structure shall be presented in ascending order by
 * `.objectID`. If NDEBUG is not defined, the inteface will perform
 * validation and assert on any inconsistencies.
 *
 * @param[in] ascendingObjectList pointer to object list (in ascending order by `.objectID`)
 * @param[in] objectListSize number of entries in ascendingObjectList
 * 
 * */
void PR6_ServerObjectInit(const struct list_element *ascendingObjectList, uint32_t objectListSize);

/**
 * Lookup and call a method
 *
 * @param[in] r server instance
 * @param[in] arg object information, argument and buffer for returnValue
 * @param[out] out result invocation result
 *
 * @return #pr05_adapter_result result of calling the adapater
 *
 * */
enum pr05_result PR6_ServerObjectInterface(const struct pr05_server *r, struct pr05_server_adapter *arg, enum pr05_result *result);

/** @} */
#endif
