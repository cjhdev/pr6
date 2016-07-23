require "test/unit"
require "wrangle/client"
require "wrangle/method_request"

class TestClient < Test::Unit::TestCase

    include Wrangle

    def test_init

        requestList = [
            MethodRequest.new(0, 0, "hello world")
        ]

        assert_raise do
            Client.new
        end

        assert_raise do
            Client.new(requestList)
        end

        Client.new(requestList) do            
        end

    end

    def test_output

        requestList = [
            MethodRequest.new(0, 0, "hello world")
        ]

        c = Client.new(requestList) do            
        end

        assert_equal("\x00\x00\x00\x00\x0bhello world".force_encoding("ASCII-8BIT"), c.output)

    end

    def test_output_tooShort

        requestList = [
            MethodRequest.new(0, 0, "hello world")
        ]

        c = Client.new(requestList) do            
        end

        assert_equal("", c.output(15))
    
    end

    def test_output_nc

        requestList = [
            MethodRequest.new(0, 0, "hello world")
        ]
    
        c = Client.new(requestList, confirmed: false) do            
        end

        assert_equal("\x01\x00\x00\x00\x0bhello world".force_encoding("ASCII-8BIT"), c.output)
    
    end

    def test_output_boe

        requestList = [
            MethodRequest.new(0, 0, "hello world")
        ]

        c = Client.new(requestList, breakOnError: true) do            
        end

        assert_equal("\x03\x00\x00\x00\x0bhello world".force_encoding("ASCII-8BIT"), c.output)
    
    end

    def test_output_nc_boe

        requestList = [
            MethodRequest.new(0, 0, "hello world")
        ]

        c = Client.new(requestList, confirmed: false, breakOnError: true) do            
        end

        assert_equal("\x02\x00\x00\x00\x0bhello world".force_encoding("ASCII-8BIT"), c.output)
    
    end

    def test_input

        requestList = [
            MethodRequest.new(0, 0, "hello world")
        ]

        touch = 0

        c = Client.new(requestList) do
            touch = 1
        end

        c.output(30)

        input = "\x04\x00\x01\x00\x00".force_encoding("ASCII-8BIT")
        c.input(1, input)

        assert_equal(1, touch)
        
    end

    def test_input_wrongCounter

        requestList = [
            MethodRequest.new(0, 0, "hello world")
        ]

        touch = 0

        c = Client.new(requestList) do
            touch = 1
        end

        input = "\x04\x00\x02\x00\x00".force_encoding("ASCII-8BIT")
        c.input(1, input)

        assert_equal(0, touch)

        input = "\x04\x00\x00\x00\x00".force_encoding("ASCII-8BIT")
        c.input(1, input)

        assert_equal(0, touch)

    end

    def test_timeout

        requestList = [
            MethodRequest.new(0,1,"hello world"),
            MethodRequest.new(2,3,"hello world")
        ]

        touch = 0

        thisTest = self

        c = Client.new(requestList, receiver: thisTest) do |res|

            touch = 1
                
            assert_equal(2, res.size)

            assert_equal(0, res.first.objectID)
            assert_equal(1, res.first.methodIndex)
            assert_equal("hello world", res.first.argument)
            assert_equal(:PR6_CLIENT_RESULT_TIMEOUT, res.first.result)
            assert_equal(nil, res.first.returnValue)
            
            assert_equal(2, res.last.objectID)
            assert_equal(3, res.last.methodIndex)
            assert_equal("hello world", res.last.argument)
            assert_equal(:PR6_CLIENT_RESULT_TIMEOUT, res.last.result)
            assert_equal(nil, res.last.returnValue)

        end
                
        assert_equal(0, touch)

        c.timeout

        assert_equal(1, touch)

    end

end
