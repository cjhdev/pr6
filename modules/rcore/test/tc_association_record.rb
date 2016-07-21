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
require "wrangle/association_record"
require 'sequel'
require 'logger'

class TestAssociationRecord < Test::Unit::TestCase

    include Wrangle

    def setup
        Sequel.extension :migration
        #DB.association = Sequel.sqlite '', :loggers => [Logger.new($stdout)]
        DB.association = Sequel.sqlite
        #DB.association.sql_log_level = :debug
        Sequel::Migrator.run(DB.association, "../../modules/rcore/db/migrations")    
    end

    def test_create

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        AssociationRecord.create(entityID, localID, remoteID)

    end

    def test_read

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        test_create

        a = AssociationRecord.read(entityID, localID, remoteID)

        assert_not_equal(nil, a)

    end

    def test_update

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        test_create

        a = AssociationRecord.read(entityID, localID, remoteID)

        assert_not_equal(nil, a)

        assert_equal(false, a.evaluateRole(:test))

        a.assignRole(:test)

        AssociationRecord.update(a)

        a = AssociationRecord.read(entityID, localID, remoteID)

        assert_equal(true, a.evaluateRole(:test))

    end

    def test_delete

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        test_create

        AssociationRecord.delete(entityID, localID, remoteID)

        assert_equal(nil, AssociationRecord.read(entityID, localID, remoteID))

        AssociationRecord.delete(entityID, localID, remoteID)

    end


    def test_outputCounter

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        test_create

        assert_equal(1, AssociationRecord.outputCounter(entityID, remoteID))
        assert_equal(2, AssociationRecord.outputCounter(entityID, remoteID))

        a = AssociationRecord.read(entityID, localID, remoteID)
        a.invocationCounter = 0xffffffff
        AssociationRecord.update(a)

        assert_equal(nil, AssociationRecord.outputCounter(entityID, remoteID))

    end

    def test_inputCounter

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        test_create

        assert_equal(false, AssociationRecord.inputCounter(entityID, localID, remoteID, 0))
        assert_equal(true, AssociationRecord.inputCounter(entityID, localID, remoteID, 1))
        assert_equal(true, AssociationRecord.inputCounter(entityID, localID, remoteID, 2))
        assert_equal(false, AssociationRecord.inputCounter(entityID, localID, remoteID, 2))
        assert_equal(false, AssociationRecord.inputCounter(entityID, localID, remoteID, 1))

    end


    def test_inputCounter_window2

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        AssociationRecord.create(entityID, localID, remoteID, remoteInvocationCounterWindow: 2)

        assert_equal(true, AssociationRecord.inputCounter(entityID, localID, remoteID, 1))
        assert_equal(true, AssociationRecord.inputCounter(entityID, localID, remoteID, 3))
        assert_equal(true, AssociationRecord.inputCounter(entityID, localID, remoteID, 2))        
        assert_equal(true, AssociationRecord.inputCounter(entityID, localID, remoteID, 4))        
        assert_equal(false, AssociationRecord.inputCounter(entityID, localID, remoteID, 1))        

    end
    
end

