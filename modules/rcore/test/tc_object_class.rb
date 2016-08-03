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
require "wrangle/object_class"

class TestObjectClass < Test::Unit::TestCase

    include Wrangle

    class MyClass < Wrangle::ObjectClass

        m "methodOne"
        m "exception"
        m "argument"
        m "temporary"
        m "permanent"
         
    end

    class MockServer

    end

    def setup

        @myObject = MyClass.new(22) do

            m("methodOne", role: [:public]) { |arg| arg }
            m("exception", role: [:public]) { raise "this exception" }
            m("argument", role: [:public]) { haltForArgument }
            m("temporary", role: [:public]) { haltForTemporary }
            m("permanent", role: [:public]) { haltForPermanent }
        
        end

        @caller = MockServer.new
        
    end

    def test_request

        assignedRole = [:public]

        result = @myObject.invoke(@caller, assignedRole, 0, "hello world")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_SUCCESS, result[:result])
        assert_equal("hello world", result[:returnValue])

        result = @myObject.invoke(@caller, assignedRole, 0, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_SUCCESS, result[:result])
        assert_equal("", result[:returnValue])
        
    end

    def test_methodUndefined

        assignedRole = [:public]

        result = @myObject.invoke(@caller, assignedRole, 255, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_METHOD_UNDEFINED, result[:result])

    end

    def test_methodAccessDenied

        assignedRole = [:otherRole]

        result = @myObject.invoke(@caller, assignedRole, 0, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_ACCESS_DENIED, result[:result])

    end

    def test_methodArgument

        assignedRole = [:public]

        result = @myObject.invoke(@caller, assignedRole, 2, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_ARGUMENT, result[:result])

    end

    def test_methodPermanent

        assignedRole = [:public]

        result = @myObject.invoke(@caller, assignedRole, 3, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_TEMPORARY , result[:result])

    end

    def test_methodTemporary

        assignedRole = [:public]

        result = @myObject.invoke(@caller, assignedRole, 4, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_PERMANENT, result[:result])

    end

end
