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

require "test/unit"
require "wrangle/server"
require "wrangle/object_class"

class TestServer < Test::Unit::TestCase

    include Wrangle

    class TestClass < ObjectClass
        m "test"
    end

    class MockAssociation

        def initialize(assignedRole, remoteMax)
            @assignedRole = assignedRole
            @remoteMax = remoteMax
        end
    
        attr_reader :assignedRole, :remoteMax
        
    end

    def setup

        myObj = TestClass.new(0) do
            m("test", role: [:public]) do |argument|
                argument 
            end            
        end

        @objects = {}
        @objects[myObj.objectID] = myObj

        @myServer = Server.new(MockAssociation.new([:public], 0), @objects)        

    end

    def test_server_success

        input = "\x00\x00\x00\x00\x05hello".force_encoding("ASCII-8BIT")
        inputCounter = 1
        output = "\x04\x00\x01\x00\x05hello".force_encoding("ASCII-8BIT")

        assert_equal(output, @myServer.input(inputCounter, input))

    end

    def test_server_objectUndefined

        input = "\x00\x00\x01\x00\x05hello".force_encoding("ASCII-8BIT")
        inputCounter = 1
        output = "\x04\x00\x01\x01".force_encoding("ASCII-8BIT")

        assert_equal(output, @myServer.input(inputCounter, input))

    end

    def test_server_methodUndefined

        input = "\x00\x00\x00\x01\x05hello".force_encoding("ASCII-8BIT")
        inputCounter = 1
        output = "\x04\x00\x01\x02".force_encoding("ASCII-8BIT")

        assert_equal(output, @myServer.input(inputCounter, input))

    end

    def test_server_accessDenied

        @myServer = Server.new(MockAssociation.new([:other], 0), @objects)   

        input = "\x00\x00\x00\x00\x05hello".force_encoding("ASCII-8BIT")
        inputCounter = 1
        output = "\x04\x00\x01\x03".force_encoding("ASCII-8BIT")

        assert_equal(output, @myServer.input(inputCounter, input))

    end

end
