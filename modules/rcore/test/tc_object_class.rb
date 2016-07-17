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

    class MyClassCID002A < Wrangle::ObjectClass

        defineMethod("methodOne")
        defineMethod("exception")
        defineMethod("argument")
        defineMethod("temporary")
        defineMethod("permanent")
        defineMethod("pause")
         
    end

    class MockAssociation

        attr_accessor :assignedRole

    end

    class MockServer

        attr_accessor :association

    end

    def setup

        @myObject = MyClassCID002A.new(22, "MyObject") do

            defineMethodHandler("methodOne", [:public]) { |arg| arg }
            defineMethodHandler("exception", [:public]) { raise "this exception" }
            defineMethodHandler("argument", [:public]) { haltForArgument }
            defineMethodHandler("temporary", [:public]) { haltForTemporary }
            defineMethodHandler("permanent", [:public]) { haltForPermanent }
            defineMethodHandler("pause", [:public]) { pause }
        
        end

        @caller = MockServer.new
        @caller.association = MockAssociation.new
        @caller.association.assignedRole = [:public]

    end

    def test_request

        result = @myObject.callMethod(@caller, 0, "hello world")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_SUCCESS, result[:result])
        assert_equal("hello world", result[:returnValue])

        result = @myObject.callMethod(@caller, 0, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_SUCCESS, result[:result])
        assert_equal("", result[:returnValue])
        
    end

    def test_methodUndefined

        result = @myObject.callMethod(@caller, 255, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_METHOD_UNDEFINED, result[:result])

    end

    def test_methodAccessDenied

        @caller.association.assignedRole = [:otherRole]

        result = @myObject.callMethod(@caller, 0, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_ACCESS_DENIED, result[:result])

    end

    def test_methodArgument

        result = @myObject.callMethod(@caller, 2, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_ARGUMENT, result[:result])

    end

    def test_methodPermanent

        result = @myObject.callMethod(@caller, 3, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_TEMPORARY , result[:result])

    end

    def test_methodTemporary

        result = @myObject.callMethod(@caller, 4, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_SUCCESS, result[:adapterResult])
        assert_equal(:PR6_RESULT_PERMANENT, result[:result])

    end

    def test_methodYield

        result = @myObject.callMethod(@caller, 5, "")
        assert_equal(Hash, result.class)
        assert_equal(:PR6_ADAPTER_YIELD, result[:adapterResult])

    end


end
