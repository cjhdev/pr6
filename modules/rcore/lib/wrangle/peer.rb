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

require 'wrangle/ext_wrangle'
require 'wrangle/client_job'
require 'wrangle/association_record'
require 'time'
require 'wrangle/eui64_pair'

module Wrangle

    class Peer

        DEFAULT_TICK = 1
        DEFAULT_WORKER_POOL_SIZE = 1
        DEFAULT_MAX_INPUT = 500

        attr_reader :entityID

        def running?
            @running
        end

        def initialize(entityID, objects, **opts)

            @entityID = EUI64.new(entityID).to_s

            @objects = objects.to_h
            @objects.each do |k,v|
                if k != v.objectID
                    raise
                end
            end
            
            @running = false        
            @threadPool = []
            @processTimeThread = nil

            @processMessageQueue = Queue.new
            @processJobQueue = Queue.new
            @outputQueue = Queue.new
            
            @jobs = []

            @maxProcessMessageQueue = DEFAULT_MAX_INPUT
            
        end

        # start threads
        def start

            if !running?

                @processTimeThread = Thread.new { processTick }
                @threadPool << @processTimeThread

                @processJobThread = Thread.new { processJob }
                @threadPool << @processJobThread

                workers = DEFAULT_WORKER_POOL_SIZE
                while workers > 0  do
                    workers -= 1
                    @threadPool << Thread.new { processMessage }
                end

            end

        end

        # stop threads and cancel jobs
        def stop

            @threadPool.each do |t|

                t.terminate

            end

            @threadPool = []

            @running = false

        end

        # make a request
        def request(remoteID, requests, caller, **opts)
            
            job = ClientJob.new(@entityID, remoteID, requests, caller, opts)

            if job

                @processJobQueue.push({:type=>:newJob,:job=>job})
                
            else

                raise "todo callback to caller"

            end
               
        end

        # @param messageHash [Hash]
        def input(message)

            if @processMessageQueue.size < @maxProcessMessageQueue
                @processMessageQueue.push({:type => :receive, :messageHash => messageHash})
            else
                STDERR.puts "dropping input"
            end

            self

        end

        # @param non_block [true,false] if true, poll queue
        # @return [Hash]
        def output(non_block=false)

            @outputQueue.pop(non_block)
            
        end

        # @!method unpackMessage(msg)
        #
        # @param entityID [String] EUI64
        # @param msg [String]
        # @return [String, String, String, Symbol, Integer] payload, localID, remoteID, recipient, counter
        # @return [nil]

        # @!method packMessage(msg)
        #
        # @param remoteID [String] EUI64
        # @param msg [String]
        # @return [Hash]
        # @return [nil]

        ################################################################
        private

            # periodically notify the processJobThread to handle timeouts
            def processTick

                loop do

                    sleep(DEFAULT_TICK)
                    @processJobQueue.push({:type => :notifyTick})

                end
                
            end

            # handle jobs
            def processJob

                loop do

                    input = @processJobQueue.pop

                    time = Time.now

                    case input[:type]
                    when :notifySend

                        id = input[:id]
                        counter = input[:counter]
                        sendTime = input[:time]
                        
                        job = @jobs.detect do |j|
                            j.id == id
                        end
                        
                        if job
                            job.registerSent(sendTime, counter)
                        end

                    when :notifyTick
                        
                        STDERR.puts "tick-tock"

                    when :processMessage

                        STDERR.puts "got message"


                    when :newJob

                        job = input[:job]

                        @jobs << job

                        @processMessageQueue.push({
                            :type=>:send,
                            :id=>job.id,
                            :to=>job.remoteID,
                            :from=>job.localID, 
                            :message=>job.sendMessage
                        })

                    else
                        raise
                    end
                                            
                end

            end

            # handle pack/unpack of all PR6 messages
            def processMessage

                loop do

                    input = @processMessageQueue.pop

                    case message[:type]
                    when :send

                        id = message[:id]
                        to = message[:to]
                        from = message[:from]
                        
                        output = packMessage(from, to, message[:message])

                        if output
                    
                            @outputQueue.push({
                                :data => output[:data],
                                :to => to,
                                :from => from,
                            })

                            @processJobQueue.push({
                                :type=>:notifySend,
                                :id=>id,
                                :time => Time.now,
                                :counter => output[:counter]
                            })

                        else

                            STDERR.puts "could not send"

                        end

                    when :receive

                        result = unpackMessage(messageHash[:data])

                        if result

                            case result[:recipient]
                            when :client

                                @processJobQueue.push({
                                    :type => :processMessage,
                                    :from => result[:from],
                                    :to => result[:to],
                                    :message => counter
                                })

                            when :server

                                association = AssociationRecord.read(result[:to], result[:to], result[:from])

                                if association

                                    output = PackMessage(result[:to], Server.new(association).input(result[:counter], result[:data]).output)
                                            
                                    if output.size > 0

                                        @output.push({
                                            :to => result[:from],
                                            :from => result[:to],
                                            :data => output
                                        })

                                    end

                                else

                                    STDERR.puts "association lost"

                                end

                            end

                        end

                    else

                        STDERR.puts "dropping message: no association for #{messageHash[:to]}:#{messageHash[:from]}"

                    end

                end

            end

    end

end
