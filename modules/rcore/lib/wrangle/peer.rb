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
require 'wrangle/peer'

module Wrangle

    class Peer

        DEFAULT_TICK = 1
        DEFAULT_WORKER_POOL_SIZE = 1
        DEFAULT_MAX_INPUT = 500

        attr_reader :entityID

        def running?
            @running
        end

        def initialize(entityID, **opts)

            @entityID = EUI64.new(entityID).to_s

            @objects = {}

            if opts[:objectList]
                opts[:objectList].each do |o|
                    @objects[o.objectID] = o
                end
            end
            
            @running = false        
            @threadPool = []
            @processTimeThread = nil

            @processMessageQueue = Queue.new
            @processJobQueue = Queue.new
            @outputQueue = Queue.new

            @ids = []
            @jobs = {}
            @preJobs = {}
            @timeoutList = []

            @maxProcessMessageQueue = DEFAULT_MAX_INPUT
            
        end

        # start threads
        def start

            if !@running

                @processTimeThread = Thread.new { processTick }
                @threadPool << @processTimeThread

                @processJobThread = Thread.new { processJob }
                @threadPool << @processJobThread

                workers = DEFAULT_WORKER_POOL_SIZE
                while workers > 0  do
                    workers -= 1
                    @threadPool << Thread.new { processMessage }                    
                end

                @running = true

            end

            self

        end

        # stop threads and cancel jobs
        def stop

            @threadPool.each do |t|

                t.terminate

            end

            @threadPool = []

            @running = false

            @jobs.each do |j|
                j.signalCancel                
            end

            @jobs = []
            @timeoutList = []
            @preJobs = {}
            @messageIndex = {}

        end

        # make a request
        def request(remoteID, **opts, &requests)

            # raise any exceptions now
            responseQueue = Queue.new
            ClientJob.new(@entityID, remoteID, responseQueue, opts, &requests)

            @processJobQueue.push({
                :type=>:newJob,
                :remoteID=>remoteID,
                :requests=>requests,
                :responseQueue=>responseQueue,
                :opts=>opts
            })

            return responseQueue
   
        end

        # deliver message to Peer
        def input(message, **opts)
            
            @processMessageQueue.push({
                :type => :receive,
                :ip => opts[:ip],
                :port => opts[:port],
                :data => message
            })            

            self

        end

        # get output message from Peer
        # @param non_block [true,false] if true, poll queue
        def output(non_block=false)
            @outputQueue.pop(non_block)            
        end


        ################################################################
        private

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


            # periodically notify the processJobThread to handle timeouts
            def processTick

                loop do

                    sleep(DEFAULT_TICK)
                    @processJobQueue.push({:type => :notifyTick})

                end
                
            end

            # process jobs
            # this thread "owns" @jobs
            def processJob

                loop do

                    input = @processJobQueue.pop

                    time = Time.now

                    case input[:type]
                    # notify job that message was not sent
                    when :notifyNotSent

                        Log::debug "#{__method__}: #{input}"

                        id = input[:id]
                        sendTime = input[:time]

                        preJob = @preJobs[id]

                        if preJob.nil?
                            raise "missing preJob"
                        end

                        # remove from pre-job list
                        @preJobs.delete(id)

                        # notify application of failure
                        preJob.forceCancel

                        # remove id from list
                        @ids.delete(preJob.id)

                        Log::info("#{__method__}: failed to send job #{preJob.id} at #{sendTime}")
                        
                    # notify job that message has been packed
                    when :notifyPacked

                        Log::debug "#{__method__}: #{input}"

                        id = input[:id]
                        counter = input[:counter]
                        sendTime = input[:time]
                        
                        preJob = @preJobs[id]

                        if preJob.nil?
                            raise "missing preJob"
                        end

                        # register the send
                        preJob.outputConfirm(counter, sendTime)

                        # remove from pre-job list
                        @preJobs.delete(id)

                        # this is how jobs are indexed in @job list
                        index = "#{preJob.assocID}-#{counter}"

                        # this should be exceedingly rare - delete both jobs to resolve ambiguity
                        # todo: explain exactly what this situation is
                        if @jobs[index]

                            collision = @jobs[index]
                            
                            # remove collision from @jobs list
                            @jobs.delete(index)
                            
                            # notify application that both jobs are cancelled
                            collision.forceCancel
                            preJob.force.Cancel
                            
                            # remove the ids from @ids list
                            @ids.delete(collision.id)
                            @ids.delete(preJob.id)

                        else

                            # send
                            @outputQueue.push(input[:message])

                            # add new job to the active jobs list
                            @jobs[index] = preJob
                            
                            # insert new job into @timeouts by order of timeout
                            if @timeoutList.size > 0
                                @timeoutList.each_with_index do |j, i|
                                    if j.timeout > preJob.timeout
                                        @timeoutList.insert(i, preJob)
                                        break
                                    end
                                end
                            else
                                @timeoutList << preJob
                            end
                            
                        end                           

                    # notify this thread to check for timeouts
                    when :notifyTick
                        
                        # detect timeouts
                        @timeoutList.each do |j|
                            if j.timeout <= time

                                Log::debug "#{__method__}: job #{j.id} timeout!"
                            
                                # remove from timeout list and jobs
                                @timeoutList.delete(j)
                                @jobs.delete("#{j.id}-#{j.counter}")

                                # try again?
                                if j.retry?

                                    # place in prejobs until confirmed as sent
                                    @preJobs[j.id] = j

                                    # delegate sending
                                    @processMessageQueue.push({
                                        :type=>:send,
                                        :id=>j.id,
                                        :to=>j.remoteID,
                                        :from=>j.localID, 
                                        :message=>j.output,
                                        :ip =>j.ip,
                                        :port=>j.port
                                    })

                                # shut it down
                                else
                                    j.forceTimeout
                                    @ids.delete(j.id)
                                end
                                
                            else
                                break
                            end
                        end

                    # give an inbound message to a client job
                    when :processMessage

                        Log::debug "#{__method__}: #{input}"

                        to = input[:to]
                        from = input[:from]
                        message = input[:message]

                        assocID = EUI64.pair(to, from)
                        counter = Client.peekCounter(message)

                        index = "#{assocID}-#{counter}"

                        job = @jobs[index]

                        if job

                            job.receiveMessage(message)

                            if job.finished?

                                # remove from timeout list and jobs
                                @timeoutList.delete(j)
                                @jobs.delete(index)

                            end
                            
                        end
                            
                    # insert a new job into the queue
                    when :newJob

                        Log::debug "#{__method__}: #{input}"

                        preJob = nil

                        # however unlikely, ensure the same id doesn't occur twice
                        begin
                        
                            preJob = ClientJob.new(
                                @entityID,
                                input[:remoteID],
                                input[:responseQueue],
                                input[:opts],
                                &input[:requests]                                
                            )

                        end while @ids.include? preJob.id

                        # make a note of the id
                        @ids << preJob.id                        

                        # place in prejobs until confirmed as sent
                        @preJobs[preJob.id] = preJob

                        # delegate sending
                        @processMessageQueue.push({
                            :type=>:send,
                            :id=>preJob.id,
                            :to=>preJob.remoteID,
                            :from=>preJob.localID, 
                            :message=>preJob.output,
                            :ip => preJob.ip,
                            :port => preJob.port
                        })

                    else
                        raise "unknow message type"
                    end
                                            
                end

            end

            # handle pack/unpack of all PR6 messages
            def processMessage

                loop do

                    input = @processMessageQueue.pop

                    Log::debug "#{__method__}: #{input}"

                    case input[:type]
                    when :send

                        id = input[:id]
                        to = input[:to]
                        from = input[:from]

                        output = packMessage(to, input[:message])

                        if output
                    
                            @processJobQueue.push({
                                :type=>:notifyPacked,
                                :id=>id,
                                :time => Time.now,
                                :counter => output[:counter],
                                :message => {
                                    :to => to,
                                    :from => from,
                                    :message => output[:data],
                                    :ip => input[:ip],
                                    :port => input[:port]
                                }                                
                            })

                        else

                            @processJobQueue.push({
                                :type=>:notifyNotSent,
                                :id=>id,
                                :time => Time.now
                            })

                        end

                    when :receive

                        id = input[:id]
                        
                        result = unpackMessage(input[:data])

                        if result

                            case result[:recipient]
                            when :PR6_RECIPIENT_CLIENT

                                @processJobQueue.push({
                                    :type => :processMessage,
                                    :from => result[:from],
                                    :to => result[:to],
                                    :message => result[:data]                                    
                                })

                            when :PR6_RECIPIENT_SERVER

                                association = AssociationRecord.read(result[:to], result[:to], result[:from])

                                if association

                                    output = packMessage(result[:from], Server.new(association, @objects).input(result[:counter], result[:data]))
                                            
                                    if output.size > 0

                                        @outputQueue.push({
                                            :to => result[:from],
                                            :from => result[:to],
                                            :ip => input[:ip],
                                            :port => input[:port],
                                            :message => output[:data]
                                        })

                                    end

                                else

                                    Log::warning "#{__method__}: assocation record removed since last read"
    
                                end

                            else
                                raise
                            end

                        end

                    else
                        raise "unknown message type"
                    end

                end

            end

    end

end
