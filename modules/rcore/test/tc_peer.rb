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
require "wrangle/peer"
require 'logger'

class TestPeer < Test::Unit::TestCase

    include Wrangle

    ENTITY_ID = "00-00-00-00-00-00-00-01"
    LOCAL_ID = "00-00-00-00-00-00-00-01"
    REMOTE_ID = "00-00-00-00-00-00-00-ff"

    def setup

        Sequel.extension :migration        
        db = Sequel.sqlite        
        Sequel::Migrator.run(db, "#{Dir.pwd}/db/migrations")
        AssociationRecord.db = db
        AssociationRecord.create(ENTITY_ID, LOCAL_ID, REMOTE_ID)
        Thread.abort_on_exception = true
        
    end

    def test_init

        Peer.new(ENTITY_ID)
        
    end

    def test_start
        peer = Peer.new(ENTITY_ID)
        peer.start
        assert(peer.running?)
    end

    def test_stop
        peer = Peer.new(ENTITY_ID)
        peer.start
        assert(peer.running?)
        peer.stop
        assert(!peer.running?)
    end

    
    def test_request

        peer = Peer.new(ENTITY_ID).start

        peer.request(REMOTE_ID) do
            request(0,0,"hello")
        end

        peer.output
            
    end

    
end
