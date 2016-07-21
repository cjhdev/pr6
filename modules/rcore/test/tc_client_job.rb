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

        c = ClientJob.new(LOCAL_ID, REMOTE_ID, [{:objectID=>0,:methodID=>0,:argument=>"hello world"}], nil)

        assert_equal(false, c.active?)

        assert_equal(LOCAL_ID, c.localID)
        assert_equal(REMOTE_ID, c.remoteID)
        assert_equal(nil, c.timeout)

    end

    def test_sendMessage

        c = ClientJob.new(LOCAL_ID, REMOTE_ID, [{:objectID=>0,:methodID=>0,:argument=>"hello world"}], nil)

        assert_equal("\x00\x00\x00\x00\x0bhello world", c.sendMessage(0))

        assert_equal(false, c.active?)
        
    end

    def test_registerSent

        c = ClientJob.new(LOCAL_ID, REMOTE_ID, [{:objectID=>0,:methodID=>0,:argument=>"hello world"}], nil)
        c.sendMessage(0)
        time = Time.now

        c.registerSent(1, time)

        assert_equal(true, c.active?)
        assert_equal(time, c.timeout)
        
    end

    def test_receiveMessage

        c = ClientJob.new(LOCAL_ID, REMOTE_ID, [{:objectID=>0,:methodID=>0,:argument=>"hello world"}], nil)
        c.sendMessage(0)
        time = Time.now
        c.registerSent(1, time)

        c.receiveMessage("\x04\x00\x01\x01".force_encoding("ASCII-8BIT"))

    end
    
end
