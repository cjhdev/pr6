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

require 'wrangle/eui64_pair'
require 'wrangle/association'
require 'set'
require 'sequel'
require 'base64'
require 'json'

module Wrangle

    class AssociationRecord

        def self.db=(db)
            @db = db
        end

        def self.create(entityID, localID, remoteID, **opts)
        
            a = Association.new(entityID, localID, remoteID, opts)

            @db[:associations].insert(
                entityID: a.entityID,
                localID: a.localID,
                remoteID: a.remoteID,
                assocID: a.assocID,
                remoteMax: a.remoteMax,
                assignedRole: JSON.dump(a.assignedRole.to_a),
                secret: Base64.encode64(a.secret),
                remoteInvocationCounter: JSON.dump(a.remoteInvocationCounter.to_a),
                remoteInvocationCounterWindow: a.remoteInvocationCounterWindow,
                invocationCounter: a.invocationCounter
            )
            a                
        end

        def self.read(entityID, localID, remoteID)
            record = @db[:associations].select.where(entityID: EUI64.new(entityID).to_s, assocID: EUI64.pair(localID, remoteID).to_s).first
            if record
                Association.new(
                    entityID,
                    localID,
                    remoteID,
                    remoteMax: record[:remoteMax],
                    assignedRole: JSON.load(record[:assignedRole]),
                    secret: Base64.decode64(record[:secret]),
                    remoteInvocationCounter: JSON.load(record[:remoteInvocationCounter]),
                    invocationCounter: record[:invocationCounter],
                    remoteInvocationCounterWindow: record[:remoteInvocationCounterWindow]
                )                
            else
                nil
            end            
        end

        def self.update(a)
            @db[:associations].where(entityID: a.entityID, assocID: a.assocID).update(
                remoteMax: a.remoteMax,
                assignedRole: JSON.dump(a.assignedRole.to_a),
                secret: Base64.encode64(a.secret),
                remoteInvocationCounter: JSON.dump(a.remoteInvocationCounter.to_a),
                remoteInvocationCounterWindow: a.remoteInvocationCounterWindow,
                invocationCounter: a.invocationCounter
            )
            true
        end
        
        def self.delete(entityID, localID, remoteID)
            a = Association.new(entityID, localID, remoteID)
            @db[:associations].where(entityID: a.entityID, assocID: a.assocID).delete
            true
        end

        def self.inputCounter(entityID, localID, remoteID, counter)
            result = false
            @db.transaction do
                a = read(entityID, localID, remoteID)
                if a.inputCounter(counter)
                    @db[:associations].where(entityID: a.localID, assocID: a.assocID).update(remoteInvocationCounter: JSON.dump(a.remoteInvocationCounter.to_a))
                    result = true
                end
            end
            result
        end
           
        def self.outputCounter(entityID, remoteID)
            counter = nil
            @db.transaction do
                a = read(entityID, entityID, remoteID)
                counter = a.outputCounter
                if counter
                    @db[:associations].where(entityID: a.entityID, assocID: a.assocID).update(invocationCounter: a.invocationCounter)
                end
            end
            counter
        end
           
        def self.encryptGCM(entityID, remoteID, iv, text, aad)
            read(entityID, localID, remoteID).encryptGCM(iv, text, aad)
        end

        def self.decryptGCM(entityID, localID, remoteID, iv, text, aad, mac)
            read(entityID, localID, remoteID).decryptGCM(iv, text, aad, mac)
        end
        
    end

end

