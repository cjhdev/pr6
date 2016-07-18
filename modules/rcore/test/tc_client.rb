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

    
    
end
