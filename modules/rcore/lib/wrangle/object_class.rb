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

require 'wrangle/constants'

module Wrangle

    class MethodHandlerResult < Exception
    end
    
    class ObjectClass

        VERSIONED_NAME_REGEX = /^(?<name>([A-Z][A-Za-z0-9]*))V(?<version>[0-9]+)$/
        NAME_REGEX = /^(?<name>([A-Z][A-Za-z0-9]*))$/

        def self.inherited(subclass)            
            subclass.defineClass
        end

        # used to initialise an ObjectClass subclass
        def self.defineClass
            name = self.name.split('::').last
            match = VERSIONED_NAME_REGEX.match(name)

            if match
            
                @classVersion = match[:version].to_i
                @className = match[:name].to_s

            else

                match = NAME_REGEX.match(name)

                if match
                    @classVersion = 0
                    @className = match[:name].to_s
                else
                    raise "class name '#{name}' must be meet requirements"
                end

            end

            if @classVersion > CLASS_VERSION_MAX
                raise ArgumentError.new "classVersion must be an Integer in the range (0..#{CLASS_VERSION_MAX})"
            end

            @methods = {}       
        end

        # Attach a text description to the class
        #
        # @param description [String] description of class
        # @return [String]
        def self.desc(description)
            @description = description
        end

        # @return [String] Class Name
        def self.className
            @className
        end

        # @return [Integer] Class Version
        def self.classVersion
            @classVersion
        end

        # @param index [Integer,String] Method Index or Method Name
        # @return [Hash] method parameters
        def self.definedMethods(index)
            if index.kind_of? Integer
                @methods.values.detect do |v|
                    v[:methodIndex] == index
                end
            else
                @methods[index.to_s]
            end
        end

        # @return [Integer] Object Identifier for this object
        attr_reader :objectID

        # @return [String] Class Name
        def className
            self.class.className
        end  

        # @return [Integer] Class Version
        def classVersion
            self.class.classVersion
        end

        # @param objectID [Integer] unique ObjectIdentifier
        # @param objectName [String, nil] optional human readable name for instance
        def initialize(objectID, **opts, &handlers)

            if !Range.new(0, OBJECT_ID_MAX).include? objectID.to_i
                raise ArgumentError
            end            
            @objectID = objectID.to_i
            @methods = {}
            self.instance_eval(&handlers)
            
        end

        # Invoke a method
        #
        # @param caller [Server] the calling server instance
        # @param assignedRole [Array] the role assigned to the caller
        # @param methodIndex [Integer] method identifier
        # @param argument [String] method invocation argument
        #
        # @return adapterResult [Hash]
        #
        # @option adapterResult [Symbol] :adapterResult 
        # @option adapterResult [Symbol] :result
        # @option adapterResult [String] :returnValue
        #
        def invoke(caller, assignedRole, methodIndex, argument)
    
            if @methods[methodIndex.to_i]

                if (@methods[methodIndex.to_i][:role] & assignedRole.to_a).size > 0

                    begin

                        retval = self.instance_exec(argument, &@methods[methodIndex][:handler])
                        if retval.nil?
                            retval = ""
                        end

                        { :adapterResult => :PR6_ADAPTER_SUCCESS, :result => :PR6_RESULT_SUCCESS, :returnValue => retval }

                    rescue MethodHandlerResult => ex

                        { :adapterResult => :PR6_ADAPTER_SUCCESS, :result => ex.message.to_sym }

                    rescue

                        raise

                    end

                else

                    { :adapterResult => :PR6_ADAPTER_SUCCESS, :result => :PR6_RESULT_ACCESS_DENIED }

                end

            else

                { :adapterResult => :PR6_ADAPTER_SUCCESS, :result => :PR6_RESULT_METHOD_UNDEFINED }

            end

        end

        ################################################################
        private_class_method

            # define a method
            #
            # @param methodName [String] name of this method
            # @param opts [Hash] method definition options
            #
            # @options opts [Integer] :methodIndex optional explicit Method Identifier
            #
            def self.m(methodName, **opts)

                if @methods[methodName.to_s].nil?

                    methodIndex = opts[:methodIndex]

                    if methodIndex
                        if !Range.new(0, METHOD_INDEX_MAX).include? methodIndex.to_i
                            raise ArgumentError
                        end
                        if methods[methodIndex.to_i]
                            raise
                        end
                    else
                        if @methods.size == 0
                            methodIndex = 0
                        else
                            methodIndex = (@methods.values.map{ |v| v[:methodIndex] }.to_set ^ Range.new(0, METHOD_INDEX_MAX).to_set).first
                            if methodIndex.nil?
                                raise Exception "Implicitly allocated methodIndex is exhausted at method definition '#{methodName}'"
                            end
                        end
                    end

                    @methods[methodName.to_s] = {
                        :methodName => methodName.to_s,
                        :methodIndex => methodIndex,
                        :description => opts[:desc],
                        :argument => opts[:argument],
                        :returnValue => opts[:returnValue]
                    }

                else
                    raise ArgumentError.new "Method '#{methodName}' has already been defined for this Class"
                end

            end

            
            
        ################################################################
        private

            # define a method handler
            #
            # @param methodName [String] name of the method as defined in class
            # @param handler [Block] the method handler
            def m(methodName, **opts, &handler)

                methodDef = self.class.definedMethods(methodName.to_s)

                if methodDef

                    @methods[methodDef[:methodIndex]] = {
                        :methodIndex => methodDef[:methodIndex],
                        :methodName => methodName.to_s,
                        :handler => handler,
                        :role => opts[:role]||[]
                    }

                else
                    raise "cannot define method handler for undefined method"
                end
                
            end
            
            # Called by #methodHandler to halt execution for reason of argument format
            def haltForArgument
                raise MethodHandlerResult.new :PR6_RESULT_ARGUMENT
            end

            # Called by #methodHandler to halt execution for unspecified permament failure condition
            def haltForPermanent
                raise MethodHandlerResult.new :PR6_RESULT_PERMANENT
            end

            # Called by #methodHandler to halt execution for unspecified temporary failure condition
            def haltForTemporary
                raise MethodHandlerResult.new :PR6_RESULT_TEMPORARY
            end

    end

end
