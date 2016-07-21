require "test/unit"
require "wrangle/client"

class TestClient < Test::Unit::TestCase

    include Wrangle

    def test_init

        assert_raise do
            Client.new
        end

        assert_raise do
            Client.new do
                request 0, 0, "hello world"
            end
        end

        assert_raise do
            Client.new do
                request 0, 0, "hello world"
                response
            end
        end

        Client.new do
            request 0, 0, "hello world"
            response do                    
            end
        end

    end

    def test_output

        c = Client.new do
            request 0, 0, "hello world"
            response do                    
            end
        end

        assert_equal("\x00\x00\x00\x00\x0bhello world".force_encoding("ASCII-8BIT"), c.output)

    end

    def test_output_tooShort

        c = Client.new do
            request 0, 0, "hello world"
            response do                    
            end
        end

        assert_equal("", c.output(15))
    
    end

    def test_output_nc

        c = Client.new confirmed: false do
            request 0, 0, "hello world"
            response do                    
            end
        end

        assert_equal("\x01\x00\x00\x00\x0bhello world".force_encoding("ASCII-8BIT"), c.output)
    
    end

    def test_output_boe

        c = Client.new breakOnError: true do
            request 0, 0, "hello world"
            response do                    
            end
        end

        assert_equal("\x03\x00\x00\x00\x0bhello world".force_encoding("ASCII-8BIT"), c.output)
    
    end

    def test_output_nc_boe

        c = Client.new confirmed: false, breakOnError: true do
            request 0, 0, "hello world"
            response do                    
            end
        end

        assert_equal("\x02\x00\x00\x00\x0bhello world".force_encoding("ASCII-8BIT"), c.output)
    
    end

    def test_input

        touch = 0

        c = Client.new do
            request 0, 0, "hello world"
            response do |res|
                touch = 1                
            end            
        end

        c.output(30)

        input = "\x04\x00\x01\x00\x00".force_encoding("ASCII-8BIT")
        c.input(1, input)

        assert_equal(1, touch)
        
    end

    def test_input_wrongCounter

        touch = 0

        c = Client.new do
            request 0, 0, "hello world"
            response do |res|
                touch = 1                
            end            
        end

        input = "\x04\x00\x02\x00\x00".force_encoding("ASCII-8BIT")
        c.input(1, input)

        assert_equal(0, touch)

        input = "\x04\x00\x00\x00\x00".force_encoding("ASCII-8BIT")
        c.input(1, input)

        assert_equal(0, touch)

    end

    def test_timeout

        touch = 0

        thisTest = self

        c = Client.new do
            request 0, 1, "hello world"
            request 2, 3, "hello world"
            response(thisTest) do |res|

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
        end

        assert_equal(0, touch)

        c.timeout

        assert_equal(1, touch)

    end

end
