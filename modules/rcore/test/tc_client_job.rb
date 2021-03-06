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

require "test/unit"
require "wrangle/client_job"

class TestClientJob < Test::Unit::TestCase

    include Wrangle

    ENTITY_ID = "00-00-00-00-00-00-00-01"
    LOCAL_ID = "00-00-00-00-00-00-00-01"
    REMOTE_ID = "00-00-00-00-00-00-00-ff"

    def test_init

        c = ClientJob.new(LOCAL_ID, REMOTE_ID, Queue.new) do
            request(0,0,"hello world")
        end

        assert_equal(LOCAL_ID, c.localID)
        assert_equal(REMOTE_ID, c.remoteID)
        assert_equal(nil, c.timeout)

    end

    def test_output

        c = ClientJob.new(LOCAL_ID, REMOTE_ID, Queue.new) do
            request(0,0,"hello world")
        end

        assert_equal("\x00\x00\x00\x00\x0bhello world", c.output(0))
        
    end

    def test_outputConfirm

        c = ClientJob.new(LOCAL_ID, REMOTE_ID, Queue.new) do
            request(0,0,"hello world")
        end
        c.output(0)
        time = Time.now

        c.outputConfirm(1, time)
        
    end

    def test_receiveMessage

        c = ClientJob.new(LOCAL_ID, REMOTE_ID, Queue.new) do
            request(0,0,"hello world")
        end
        c.output(0)
        time = Time.now
        c.outputConfirm(1, time)

        c.input("\x04\x00\x01\x01".force_encoding("ASCII-8BIT"))

    end
    
end
