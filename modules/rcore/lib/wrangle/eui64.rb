module Wrangle

    class EUI64

        NIBBLE = "[0-9a-fA-F]"
        OCTET = "(#{NIBBLE}#{NIBBLE})"
        DELIM1 = OCTET + "-" + OCTET + "-" + OCTET + "-" + OCTET + "-" + OCTET + "-" + OCTET + "-" + OCTET + "-" + OCTET
        DELIM2 = OCTET + ":" + OCTET + ":" + OCTET + ":" + OCTET + ":" + OCTET + ":" + OCTET + ":" + OCTET + ":" + OCTET
        
        # @param input [String] EUI64 formatted string or octet string
        def initialize(input)

            if !input.is_a? String; raise ArgumentError.new "input must be a String not #{input.class}" end

            # assume this is a binary string containing EUI64
            if input.size == 8

                @bytes = input
                @eui = ""
                input.unpack("C8").each do |octet|

                    @eui << "%02X" % octet
                    @eui << "-"

                end

                @eui.chop!
                @msb = @bytes.bytes.first

            # assume text string
            else

                match = Regexp.new(DELIM1).match(input)

                if match.nil?

                    match = Regexp.new(DELIM2).match(input)

                end

                if match.nil?

                    raise "input '#{input}' is not an EUI64"
                
                else

                    integerArray = []

                    match.captures.each do |c|

                        integerArray << c.to_i(16)

                    end

                    @eui = input
                    @bytes = integerArray.pack("c*")
                    @msb = @bytes.bytes.first

                end

            end
        
        end

        # @param input [String]
        # @return [String]
        def self.new_to_s(input)

            self.new(input).to_s
            
        end

        # @param input [String]
        # @return [String]
        def self.new_to_bytes(input)

            self.new(input).bytes

        end

        # @return [Integer] byte string intepreted as a big-endian number
        def to_i

            @bytes.bytes.inject(0) {|rcv,i| (rcv << 8) + i}

        end
    
        # @return [String] byte string
        def bytes
            
            return @bytes

        end

        # @return [String] EUI format human readable string
        def to_s
        
            return @eui.to_s.force_encoding("ASCII-8BIT")

        end

        # @return [true] this eui64 is a multicast address
        # @return [false] this eui64 is a unicast address
        def is_multicast?

            if (@msb & 0x1) == 0x1; true else false end

        end

        # @return [true] this eui64 is locally administered
        # @return [false] this eui64 is OUI
        def is_local?

            if (@msb & 0x2) == 0x2; true else false end

        end

        # @param other [EUI64]
        # @return [true] values match
        # @return [false] values do not match
        def ==(other)

            if other.is_a? EUI64

                if @bytes == other.bytes

                    return true

                end

            end

            return false

        end

        alias eql? ==

        def hash

            @bytes.hash

        end

    end

end
