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

        attr_reader :assocID, :entityID, :localID, :remoteID, :assignedRole, :remoteMax, :secret, :inputCounter, :outputCounter, :inputCounterWindow

        def initialize(entityID, localID, remoteID, **opts)
            @assocID = EUI64.pair(localID, remoteID)
            @entityID = EUI64.new(entityID).to_s
            @localID = localID
            @remoteID = remoteID
            @remoteMax = 0
            @assignedRole = Set.new
            @secret = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00".freeze
            @inputCounterWindow = 1
            @outputCounter = 0
            @inputCounter = Set.new
            if opts[:remoteMax]
                setRemoteMax(remoteMax)
            end
            if opts[:assignedRole]
                opts[:assignedRole].to_a.each do |role|
                    assignRole(role)
                end            
            end
            if opts[:secret]
                setSecret(opts[:secret])
            end
            if opts[:inputCounterWindow]
                setInputCounterWindow(opts[:inputCounterWindow])
            end
            if opts[:inputCounter]
                setInputCounter(opts[:inputCounter])
            end
            if opts[:outputCounter]
                setOutputCounter(opts[:outputCounter])
            end            
        end

        def setRemoteMax(max)
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

        def outputOutputCounter
            if @outputCounter < 0xffffffff
                @outputCounter += 1
            else
                raise
            end
        end

        def inputInputCounter(counter)
            counter = counter.to_i
            result = false
            if counter > 0
                if (@inputCounter.size == 0) or (!@inputCounter.include?(counter) and @inputCounter.min < counter)
                    if @inputCounter.size == @inputCounterWindow
                        @inputCounter.delete(@inputCounter.min)
                    end
                    @inputCounter.add(counter)
                    result = true
                end
            end

            result
        end

        def setSecret(secret)
            if ![16, 24, 32].include? secret.to_s.size
                raise ArgumentError.new "secret must be a string 16, 24, or 32 bytes in size"
            end
            @secret = secret.to_s
        end

        def setInputCounterWindow(inputCounterWindow)
            if inputCounterWindow <= 0
                raise ArgumentError
            end
            @inputCounterWindow = inputCounterWindow        
        end

        def setInputCounter(inputCounter)
            @inputCounter = inputCounter.to_set
        end

        def setOutputCounter(outputCounter)
            if !(0..0xfffffffe).include? outputCounter
                raise ArgumentError
            end
            @outputCounter = outputCounter
        end

    end
           
end
