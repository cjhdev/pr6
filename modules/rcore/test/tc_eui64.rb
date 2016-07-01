require "test/unit"
require "wrangle/eui64"

class TestEUI64 < Test::Unit::TestCase

    include Wrangle

    def test_parse_bytes

        bytes = "\x80\x00\x00\x00\xee\x00\xee\xff".force_encoding("ASCII-8BIT")
        
        assert_equal(EUI64, EUI64.new(bytes).class)

    end

    def test_parse_bytes_to_bytes

        bytes = "\x80\x00\x00\x00\xee\x00\xee\xff".force_encoding("ASCII-8BIT")
        
        assert_equal(bytes, EUI64.new(bytes).bytes)

    end

    def test_parse_bytes_to_human

        bytes = "\x80\x00\x00\x00\xee\x00\xee\xff".force_encoding("ASCII-8BIT")
        human = "80-00-00-00-EE-00-EE-FF"
        
        assert_equal(human, EUI64.new(bytes).to_s)

    end

    def test_parse_bytes_multicast_true

        bytes = "\x81\x00\x00\x00\xee\x00\xee\xff".force_encoding("ASCII-8BIT")
        
        assert_equal(true, EUI64.new(bytes).is_multicast?)

    end

    def test_parse_bytes_multicast_false

        bytes = "\x80\x00\x00\x00\xee\x00\xee\xff".force_encoding("ASCII-8BIT")
        
        assert_equal(false, EUI64.new(bytes).is_multicast?)

    end

    def test_parse_bytes_local_true

        bytes = "\x82\x00\x00\x00\xee\x00\xee\xff".force_encoding("ASCII-8BIT")
        
        assert_equal(true, EUI64.new(bytes).is_local?)

    end

    def test_parse_bytes_local_false

        bytes = "\x80\x00\x00\x00\xee\x00\xee\xff".force_encoding("ASCII-8BIT")
        
        assert_equal(false, EUI64.new(bytes).is_local?)

    end

    def test_parse_human

        human = "80-00-00-00-EE-00-EE-FF"
        assert_equal(EUI64, EUI64.new(human).class)

    end
    
    def test_parse_human_to_bytes

        human = "80-00-00-00-EE-00-EE-FF"
        bytes = "\x80\x00\x00\x00\xEE\x00\xEE\xFF".force_encoding("ASCII-8BIT")
        assert_equal(bytes, EUI64.new(human).bytes)

    end

    def test_parse_human_to_human

        human = "80-00-00-00-EE-00-EE-FF"
        
        assert_equal(human, EUI64.new(human).to_s)

    end

    def test_parse_human_to_i1

        human = "00-00-00-00-00-00-00-01"
        integer = 1
        
        assert_equal(integer, EUI64.new(human).to_i)

    end

    def test_parse_human_to_i65537

        human = "00-00-00-00-00-01-00-01"
        integer = 65537
        
        assert_equal(integer, EUI64.new(human).to_i)

    end

    def test_hash

        human = "00-00-00-00-00-01-00-01"
        assert_equal(EUI64.new(human).hash, EUI64.new(human).hash)

    end

    def test_eql

        human = "00-00-00-00-00-01-00-01"
        assert_equal(EUI64.new(human), EUI64.new(human))

    end

end
