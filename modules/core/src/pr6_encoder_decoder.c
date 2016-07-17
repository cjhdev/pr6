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

/* includes ***********************************************************/

#include "pr6_encoder_decoder.h"
#include "debug_interface.h"

#include <stdio.h>
#include <string.h>

/* functions **********************************************************/

uint8_t PR6_GetVint(const uint8_t *in, uint16_t inLen, uint16_t *out)
{
    ASSERT((in != NULL))
    ASSERT((out != NULL))

    uint8_t retval = 0U;
    uint8_t pos = 0U;
    uint8_t bytes;

    if(inLen > 0U){

        if(in[pos] < 0x80U){

            *out = (uint16_t)in[pos];
            pos++;
            retval = pos;
        }
        else{

            bytes = in[pos] & 0x7fU;
            pos++;

            if((bytes != 0U) && (bytes <= 2U) && ((inLen - pos) >= bytes)){

                *out = 0U;

                for(; pos <= bytes; pos++){

                    *out <<= 8U;
                    *out |= (uint16_t)in[pos];
                }

                switch(bytes){
                default:

                    if(*out > 0x7fU){

                        retval = pos;
                    }
                    break;

                case 2U:

                    if(*out > 0xffU){

                        retval = pos;
                    }
                    break;                
                }
            }            
        }
    }
     
    return retval;    
}

uint8_t PR6_PutVint(uint16_t in, uint8_t *out, uint16_t outMax)
{
    uint8_t retval = 0U;
    uint8_t encodedSize;

    encodedSize = PR6_SizeofVint(in);

    if(encodedSize <= outMax){

        if(encodedSize > 1U){

            out[0U] = (uint8_t)(0x80U | ((encodedSize - 1U) & 0x7fU));
        }

        switch(encodedSize){
        case 1:

            out[0U] = (uint8_t)in;
            break;
            
        case 2:

            out[1U] = (uint8_t)in;
            break;
        
        case 3:
        default:

            out[1U] = (uint8_t)((in >> 8U) & 0xffU);
            out[2U] = (uint8_t)in;
            break;        
        }

        retval = encodedSize;
    }        
    
    return retval;    
}

uint8_t PR6_CastTag(uint8_t in, enum pr6_tag *out)
{
    static const enum pr6_tag tags[] = {
        PR6_METHOD_REQ,
        PR6_METHOD_NC_REQ,
        PR6_METHOD_NC_BOE_REQ,
        PR6_METHOD_BOE_REQ,
        PR6_METHOD_RES
    };

    uint8_t retval = 0U;

    if(in < (sizeof(tags) / sizeof(*tags))){

        *out = tags[in];
        retval = 1U;
    }
    else{

        DEBUG("invalid result code")
    }

    return retval;
}

uint8_t PR6_SizeofVint(uint16_t in)
{
    uint8_t retval;

    if(in < 0x80U){
        retval =  1U;
    }
    else if(in < 0x100U){
        retval = 2U;
    }
    else{
        retval = 3U;
    }

    return retval;
}

