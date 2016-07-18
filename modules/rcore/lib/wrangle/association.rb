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
require 'set'
require 'openssl'

module Wrangle

    class Association

        attr_reader :assocID, :entityID, :localID, :remoteID, :assignedRole, :remoteMax, :secret, :remoteInvocationCounter, :invocationCounter, :remoteInvocationCounterWindow

        def initialize(entityID, localID, remoteID, **opts)
            @assocID = EUI64.pair(localID, remoteID)
            @entityID = EUI64.new(entityID).to_s
            @localID = localID
            @remoteID = remoteID
            @remoteMax = 0
            @assignedRole = Set.new
            @secret = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00".freeze
            @remoteInvocationCounterWindow = 1
            @invocationCounter = 0
            @remoteInvocationCounter = Set.new
            if opts[:remoteMax]
                self.remoteMax = opts[:remoteMax]
            end
            if opts[:assignedRole]
                opts[:assignedRole].to_a.each do |role|
                    assignRole(role)
                end            
            end
            if opts[:secret]
                self.secret = opts[:secret]
            end
            if opts[:remoteInvocationCounterWindow]
                self.remoteInvocationCounterWindow = opts[:remoteInvocationCounterWindow]
            end
            if opts[:remoteInvocationCounter]
                self.remoteInvocationCounter = opts[:remoteInvocationCounter]
            end
            if opts[:invocationCounter]
                self.invocationCounter = opts[:invocationCounter]
            end            
        end

        def remoteMax=(max)
            if max.to_i > 0xffff or max.to_i < 0
                raise ArgumentError
            end
            @remoteMax = max.to_i
            self
        end

        def assignRole(role)
            @assignedRole << role.to_sym
        end

        def revokeRole(role)
            @assignedRole.delete(role.to_sym)
        end

        def evaluateRole(role)
            @assignedRole.include? role
        end

        def decryptGCM(iv, text, aad, mac)

            result = nil

            cipher = OpenSSL::Cipher.new('aes-128-gcm')
            cipher.decrypt
            cipher.key = @secret
            cipher.iv = iv
            cipher.auth_data = aad
            cipher.auth_tag = mac

            begin

                decrypted = ""

                if text.size > 0

                    decrypted << cipher.update(text)

                end

                decrypted << cipher.final
                result = decrypted
                
            rescue OpenSSL::Cipher::CipherError
                STDERR.puts "could not validate input"
            end

            result
            
        end

        def encryptGCM(iv, text, aad)

            result = nil

            cipher = OpenSSL::Cipher.new('aes-128-gcm')
            cipher.encrypt
            cipher.key = @secret
            cipher.iv = iv
            cipher.auth_data = aad

            encrypted = ""

            if text.size > 0

                encrypted << cipher.update(text)

            end

            encrypted << cipher.final
            mac = cipher.auth_tag

            result = {:text => encrypted, :mac => mac}

        end

        def outputCounter
            if @invocationCounter < 0xffffffff
                @invocationCounter += 1
            else
                nil
            end
        end

        def inputCounter(counter)
            counter = counter.to_i
            result = false
            if counter > 0
                if (@remoteInvocationCounter.size == 0) or (!@remoteInvocationCounter.include?(counter) and @remoteInvocationCounter.min < counter)
                    if @remoteInvocationCounter.size == @remoteInvocationCounterWindow
                        @remoteInvocationCounter.delete(@remoteInvocationCounter.min)
                    end
                    @remoteInvocationCounter.add(counter)
                    result = true
                end
            end

            result
        end

        def secret=(s)
            if ![16, 24, 32].include? s.to_s.size
                raise ArgumentError.new "secret must be a string 16, 24, or 32 bytes in size"
            end
            @secret = s.to_s
        end

        def remoteInvocationCounterWindow=(windowSize)
            if windowSize <= 0
                raise ArgumentError
            end
            @remoteInvocationCounterWindow = windowSize
        end

        def remoteInvocationCounter=(counter)
            @remoteInvocationCounter = counter.to_set
        end

        def invocationCounter=(counter)
            if !(0..0xffffffff).include? counter.to_i
                raise ArgumentError
            end
            @invocationCounter = counter.to_i
        end

    end
           
end
