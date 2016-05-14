/* Copyright (c) 2013-2016 Cameron Harper
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
#ifndef DEBUG_INTERFACE_H
#define DEBUG_INTERFACE_H

/**
 * @defgroup debug_interface Common Debug Interface
 *
 * A collection of useful debug macros.
 *
 * @{
 * */

#ifdef DOXYGEN

    /**
     * This macro is used to wrap assert() to remove it entirely from
     * builds that don't require it or an assert function is not available on the target.
     *
     * @param[in] X assert condition
     *
     * */
    #define ASSERT(X) assert(X);

    /**
     * This macro may be defined at compile time to replace all
     * instances of `DEBUG` in the source with a platform specific
     * debug logging function.
     *
     * @see EXAMPLE_DebugLogger
     *
     * @param[in] "..." variadic arguments
     *
     * */
    #define DEBUG(...)

    /**
     * An <b>example</b> of a compile time defined debug interface called by DEBUG
     *
     * In GCC this may be defined at compile time with the `-D` switch shown below:
     * 
     *  @code
     * -D'DEBUG(...)=EXAMPLE_DebugLogger(__FUNCTION__, __VA_ARGS__);'
     * @endcode
     *
     * @param[in] function current function name
     * @param[in] fmt printf format string
     * @param[in] "..." variadic arguments
     *
     * */
    void EXAMPLE_DebugLogger(const char *function, const char *fmt, ...);

#else

    #ifdef NDEBUG

        /*lint -e(9026) Allow assert to be removed completely */
        #define ASSERT(X)    /*lint -e(9063) ASSERT(X) used to be here */;

        /*lint -e(9026)*/
        #define DEBUG(...)    /*lint -e(9063) DEBUG(...) used to be here */;

        /*lint -e(9026)*/
        #define INFO(...)   /*lint -e(9063) INFO(...) used to be here */;

    #else

        #include <assert.h>

        /*lint -e(9026) Allow assert to be removed completely */
        #define ASSERT(X) /*lint -e(9034) Call to assert */assert(X);

        #ifdef DEBUG_EXTERN_FUNCTION    
        DEBUG_EXTERN_FUNCTION
        #endif
        
        #ifndef DEBUG

            /*lint -e(9026)*/
            #define DEBUG(...)    /*lint -e(9063) DEBUG(...) used to be here */;

        #endif
        #ifndef INFO

            /*lint -e(9026)*/
            #define INFO(...)    /*lint -e(9063) INFO(...) used to be here */;

        #endif
        
    #endif

#endif

/** @} */
#endif
