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
 * */

/* includes ***********************************************************/

#include <ruby.h>

/* public functions ***************************************************/

void EXT_DebugPutErr(const char *function, const char *fmt, ...)
{
    int ret;
    VALUE rbString;
    char format[100];
    char output[200];

    if(snprintf(format, sizeof(format), "%s: debug: %s", function, fmt) > 0){

        va_list argptr;
        va_start(argptr, fmt);
        ret = vsnprintf(output, sizeof(output), format, argptr);
        va_end(argptr);

        if((ret > 0) && (ret <= sizeof(output))){

            rbString = rb_str_new((const char *)output, ret);
            rb_io_puts(1, &rbString, rb_stderr);            
        }
        else{

            rb_bug("adjust the buffer size in %s to support error messages", __FUNCTION__);
        }
    }
    else{

        rb_bug("adjust the buffer size in %s to support error messages", __FUNCTION__);
    }
}
