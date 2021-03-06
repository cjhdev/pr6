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
require 'wrangle/peer'
require 'socket'

module Wrangle

    class UDPNode

        attr_reader :port

        def initialize(entityID, **opts)

            @port = opts[:port]||nil
            @socket = UDPSocket.new
            @socket.bind("", @port)

            @port = @socket.local_address.ip_port

            @peer = Peer.new(entityID, objectList: opts[:objectList]).start

            @threadPool = []

            # receive from socket
            @threadPool << Thread.new do

                loop do

                    message, addrInfo = @socket.recvfrom(1200)
                    @peer.input(message, ip: addrInfo[2], port: addrInfo[1])

                end

            end

            # send to socket
            @threadPool << Thread.new do

                loop do

                    input = @peer.output

                    @socket.send(input[:message], 0, input[:ip], input[:port])
    
                end

            end

        end

        def request(remoteID, **opts, &requests)

            @peer.request(remoteID, **opts, &requests).pop
            
        end

    end

end


