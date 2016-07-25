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

require 'securerandom'
require 'wrangle/eui64_pair'
require 'wrangle/client'

module Wrangle

    class ClientJob

        attr_reader :id
        attr_reader :remoteID
        attr_reader :localID
        attr_reader :assocID
        attr_reader :timeout
        attr_reader :port
        attr_reader :ip
        attr_reader :response
        attr_reader :counter
        attr_reader :state
        
        DEFAULT_RETRY_MAX = 0
        DEFAULT_RETRY_PERIOD = 0
                
        def initialize(localID, remoteID, responseQueue, **opts, &requests)

            @requestList = []
            
            time = Time.now

            @id = SecureRandom.uuid

            @assocID = EUI64.pair(localID, remoteID).to_s
            @localID = EUI64.new(localID).to_s
            @remoteID = EUI64.new(remoteID).to_s
            
            @confirmed = opts[:confirmed]||true
            @breakOnError = opts[:breakOnError]||false
            @createTime = time.dup,
            @modifiedTime = time.dup
            
            @retryPeriod = opts[:retryPeriod]||DEFAULT_RETRY_PERIOD
            @retryMax = opts[:retryMax]||DEFAULT_RETRY_MAX
            @retryCount = 0
            @retryBackoff = opts[:retryBackoff]||false

            @timeout = nil

            @ip = opts[:ip]
            @port = opts[:port]

            @counter = nil
            @responseQueue = responseQueue

            self.instance_exec(&requests)

            @client = Client.new(@requestList, confirmed: opts[:confirmed], breakOnError: opts[:breakOnError], receiver: self) do |result|                
                @responseQueue.push(result)
                @finished = true
            end

            @state = @client.state

        end

        def output(remoteMax=0)
            @timeout = nil
            @counter = nil
            out = @client.output(remoteMax)
            @state = @client.state
            out
        end

        def outputConfirm(counter, time)
            @counter = counter
            @client.outputConfirm(counter)
            if !self.finished?
                if @retryBackoff
                    @timeout = time + ( @retryPeriod << @retryCount )
                else
                    @timeout = time + @retryPeriod
                end
            end
            @state = @client.state
        end

        def input(message)
            @client.input(message)
            @state = @client.state
            self
        end

        def forceTimeout
            @client.timeout
            @state = @client.state
        end

        def forceCancel
            @client.cancel
            @state = @client.state
        end

        def retry?
            @retryCount < @retryMax                
        end

        def finished?
            @finished
        end

        private

            def request(objectID, methodIndex, argument)
                @requestList.push(MethodRequest.new(objectID, methodIndex, argument))
            end

    end

end
