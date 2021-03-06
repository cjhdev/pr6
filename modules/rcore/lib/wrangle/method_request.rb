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

require 'wrangle/constants'

module Wrangle

    class MethodRequest

        # @param objectID [Integer]
        # @param methodIndex [Integer]
        # @param argument [String]
        def initialize(objectID, methodIndex, argument)

            if !Range.new(0, OBJECT_ID_MAX).include? objectID.to_i
                raise ArgumentError
            end
            if !Range.new(0, METHOD_INDEX_MAX).include? methodIndex.to_i
                raise ArgumentError
            end
            if !Range.new(0, 0xffff).include? argument.size
                raise ArgumentError
            end
            
            @objectID = objectID.to_i
            @methodIndex = methodIndex.to_i
            @argument = argument.to_s

        end

        # return [Integer]
        attr_reader :objectID

        # return [Integer]
        attr_reader :methodIndex

        # return [String]
        attr_reader :argument

    end

end


