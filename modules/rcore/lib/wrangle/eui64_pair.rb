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

            pair.to_i

        end

    end
    
end
