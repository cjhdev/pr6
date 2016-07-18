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
require "wrangle/association"
require 'set'

class TestAssociation < Test::Unit::TestCase

    include Wrangle

    def test_init_defaults

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"
        secret = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        remoteMax = 0
        assignedRole = Set.new
        remoteInvocationCounter = Set.new
        remoteInvocationCounterWindow = 1
        invocationCounter = 0

        a = Association.new(entityID, localID, remoteID)

        assert_equal(entityID, a.entityID)
        assert_equal(localID, a.localID)
        assert_equal(remoteID, a.remoteID)
        assert_equal(secret, a.secret)
        assert_equal(remoteMax, a.remoteMax)
        assert_equal(assignedRole, a.assignedRole)
        assert_equal(remoteInvocationCounter, a.remoteInvocationCounter)
        assert_equal(remoteInvocationCounterWindow, a.remoteInvocationCounterWindow)
        assert_equal(invocationCounter, a.invocationCounter)        
    end

    def test_init

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"
        secret = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01"
        remoteMax = 0
        assignedRole = [:public, :test].to_set
        remoteInvocationCounter = [0,1,4].to_set
        remoteInvocationCounterWindow = 5
        invocationCounter = 42

        a = Association.new(entityID, localID, remoteID,
            secret: secret,
            remoteMax: remoteMax,
            assignedRole: assignedRole,
            remoteInvocationCounter: remoteInvocationCounter,
            remoteInvocationCounterWindow: remoteInvocationCounterWindow,
            invocationCounter: invocationCounter
        )

        assert_equal(entityID, a.entityID)
        assert_equal(localID, a.localID)
        assert_equal(remoteID, a.remoteID)
        assert_equal(secret, a.secret)
        assert_equal(remoteMax, a.remoteMax)
        assert_equal(assignedRole, a.assignedRole)
        assert_equal(remoteInvocationCounter, a.remoteInvocationCounter)
        assert_equal(remoteInvocationCounterWindow, a.remoteInvocationCounterWindow)
        assert_equal(invocationCounter, a.invocationCounter) 
    end

    def test_setRemoteMax
        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID)

        assert_raise ArgumentError do
            a.remoteMax = 0x10000
        end
        assert_raise ArgumentError do
            a.remoteMax = -1
        end

        a.remoteMax = 42

        assert_equal(42, a.remoteMax)
                
    end

    def test_outputCounter

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID)

        assert_equal(1, a.outputCounter)
        assert_equal(2, a.outputCounter)
        assert_equal(3, a.outputCounter)

    end

    def test_inputCounter

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID)

        # never accept counter value zero
        assert_equal(false, a.inputCounter(0))

        assert_equal(true, a.inputCounter(1))
        assert_equal(false, a.inputCounter(1))
        
        assert_equal(true, a.inputCounter(3))
        assert_equal(false, a.inputCounter(2))

    end

    def test_inputCounter_outOfOrder

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID, remoteInvocationCounterWindow: 2)

        # never accept counter value zero
        assert_equal(false, a.inputCounter(0))

        assert_equal(true, a.inputCounter(1))
        assert_equal(false, a.inputCounter(1))

        assert_equal(true, a.inputCounter(3))
        assert_equal(true, a.inputCounter(2))

    end

    def test_encryptGCM

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID, secret: "\xc9\x39\xcc\x13\x39\x7c\x1d\x37\xde\x6a\xe0\xe1\xcb\x7c\x42\x3c".force_encoding("ASCII-8BIT"))

        iv = "\xb3\xd8\xcc\x01\x7c\xbb\x89\xb3\x9e\x0f\x67\xe2".force_encoding("ASCII-8BIT")
        pt = "\xc3\xb3\xc4\x1f\x11\x3a\x31\xb7\x3d\x9a\x5c\xd4\x32\x10\x30\x69".force_encoding("ASCII-8BIT")
        ct = "\x93\xfe\x7d\x9e\x9b\xfd\x10\x34\x8a\x56\x06\xe5\xca\xfa\x73\x54".force_encoding("ASCII-8BIT")
        aad = "\x24\x82\x56\x02\xbd\x12\xa9\x84\xe0\x09\x2d\x3e\x44\x8e\xda\x5f".force_encoding("ASCII-8BIT")
        mac = "\x00\x32\xa1\xdc\x85\xf1\xc9\x78\x69\x25\xa2\xe7\x1d\x82\x72\xdd".force_encoding("ASCII-8BIT")

        result = a.encryptGCM(iv, pt, aad)

        assert_equal(Hash, result.class)
        assert_equal(ct, result[:text])
        assert_equal(mac, result[:mac])
        
    end

    def test_encryptGCM_noText

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID, secret: "\x77\xbe\x63\x70\x89\x71\xc4\xe2\x40\xd1\xcb\x79\xe8\xd7\x7f\xeb".force_encoding("ASCII-8BIT"))

        iv = "\xe0\xe0\x0f\x19\xfe\xd7\xba\x01\x36\xa7\x97\xf3".force_encoding("ASCII-8BIT")
        pt = "".force_encoding("ASCII-8BIT")
        ct = "".force_encoding("ASCII-8BIT")
        aad = "\x7a\x43\xec\x1d\x9c\x0a\x5a\x78\xa0\xb1\x65\x33\xa6\x21\x3c\xab".force_encoding("ASCII-8BIT")
        mac = "\x20\x9f\xcc\x8d\x36\x75\xed\x93\x8e\x9c\x71\x66\x70\x9d\xd9\x46".force_encoding("ASCII-8BIT")

        result = a.encryptGCM(iv, pt, aad)

        assert_equal(Hash, result.class)
        assert_equal(ct, result[:text])
        assert_equal(mac, result[:mac])

    end

    def test_decryptGCM

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID, secret: "\xc9\x39\xcc\x13\x39\x7c\x1d\x37\xde\x6a\xe0\xe1\xcb\x7c\x42\x3c".force_encoding("ASCII-8BIT"))

        iv = "\xb3\xd8\xcc\x01\x7c\xbb\x89\xb3\x9e\x0f\x67\xe2".force_encoding("ASCII-8BIT")
        pt = "\xc3\xb3\xc4\x1f\x11\x3a\x31\xb7\x3d\x9a\x5c\xd4\x32\x10\x30\x69".force_encoding("ASCII-8BIT")
        ct = "\x93\xfe\x7d\x9e\x9b\xfd\x10\x34\x8a\x56\x06\xe5\xca\xfa\x73\x54".force_encoding("ASCII-8BIT")
        aad = "\x24\x82\x56\x02\xbd\x12\xa9\x84\xe0\x09\x2d\x3e\x44\x8e\xda\x5f".force_encoding("ASCII-8BIT")
        mac = "\x00\x32\xa1\xdc\x85\xf1\xc9\x78\x69\x25\xa2\xe7\x1d\x82\x72\xdd".force_encoding("ASCII-8BIT")

        result = a.decryptGCM(iv, ct, aad, mac)

        assert_equal(String, result.class)
        assert_equal(pt, result)

    end

    def test_decryptGCM_noText

        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID, secret: "\x77\xbe\x63\x70\x89\x71\xc4\xe2\x40\xd1\xcb\x79\xe8\xd7\x7f\xeb".force_encoding("ASCII-8BIT"))
        
        iv = "\xe0\xe0\x0f\x19\xfe\xd7\xba\x01\x36\xa7\x97\xf3".force_encoding("ASCII-8BIT")
        pt = "".force_encoding("ASCII-8BIT")
        ct = "".force_encoding("ASCII-8BIT")
        aad = "\x7a\x43\xec\x1d\x9c\x0a\x5a\x78\xa0\xb1\x65\x33\xa6\x21\x3c\xab".force_encoding("ASCII-8BIT")
        mac = "\x20\x9f\xcc\x8d\x36\x75\xed\x93\x8e\x9c\x71\x66\x70\x9d\xd9\x46".force_encoding("ASCII-8BIT")

        result = a.decryptGCM(iv, ct, aad, mac)

        assert_equal(String, result.class)
        assert_equal(pt, result)

    end

    def test_assignRole
        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"

        a = Association.new(entityID, localID, remoteID)

        assert_equal([].to_set, a.assignedRole)

        a.assignRole(:public)
        
        assert_equal([:public].to_set, a.assignedRole)
    end

    def test_revokeRole
        entityID = "00-00-00-00-00-00-00-01"
        localID = "00-00-00-00-00-00-00-01"
        remoteID = "00-00-00-00-00-00-00-ff"
        assignedRole = [:public, :test].to_set

        a = Association.new(entityID, localID, remoteID, assignedRole: assignedRole)

        assert_equal(assignedRole, a.assignedRole)

        a.revokeRole(:public)
        
        assert_equal([:test].to_set, a.assignedRole)
    end

end

