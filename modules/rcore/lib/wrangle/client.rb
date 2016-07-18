# Copyright (c) 2016 Cameron Harper
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#  
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
require 'wrangle/ext_wrangle'
require 'wrangle/method_request'
require 'wrangle/method_response'

module Wrangle

    class Client

        # @!method initialize( confirmed, breakOnError, reciever, &block)
        #
        # @param confirmed [true, false]
        # @param breakOnError [true, false]
        # @param receiver [Object,nil] evaluate responseHandler within context of this object
        # @param block [Block] requests and response handler
        #
        
        # @!method input(msg)
        #
        # Deliver message to client instance
        #
        # @param msg [String] message to deliver to client
        #

        # @!method output(outMax)
        #
        # Get message from client instance
        #
        # @param outMax [Integer]
        # @returns [String] message
        #
        
        # @!method uuid
        #
        # @return [String] uuid of this instance

        # @!method registerCounter(counter)
        #
        # @param counter [Integer]

    end
    
end
    
