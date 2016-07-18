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
require 'wrangle/eui64'

module Wrangle

    class EUI64

        # Cantor pairing function for two EUI64 Strings
        #
        # @param localID [String] EUI64
        # @param remoteID [String] EUI64
        # @return [Integer]
        def self.pair(localID, remoteID)

            k1 = EUI64.new(localID).to_i
            k2 = EUI64.new(remoteID).to_i

            pair = 0.5 * ( k1 + k2 ) * (k1 + k2 + 1)

            if k1 < k2

                pair + k2

            else

                pair + k1

            end

            pair.to_i.to_s

        end

    end
    
end
