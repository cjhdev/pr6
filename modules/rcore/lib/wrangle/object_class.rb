require 'wrangle/constants'
require 'wrangle/peer'

module Wrangle

    class MethodHandlerResult < Exception; end
    
    class ObjectClass

        VERSIONED_NAME_REGEX = /^(?<name>([A-Z][A-Za-z0-9]*))CID(?<classID>[0-9a-fA-F]{4})V(?<version>[0-9]+)$/
        NAME_REGEX = /^(?<name>([A-Z][A-Za-z0-9]*))CID(?<classID>[0-9a-fA-F]{4})$/

        def self.inherited(subclass)

            subclass.defineClass
            super
            
        end

        def self.defineClass

            #if !classID.is_a? Integer or classID > CLASS_ID_MAX; raise ArgumentError.new "classID must be an Integer in the range (0..#{CLASS_ID_MAX})" end

            name = self.name.split('::').last
            match = VERSIONED_NAME_REGEX.match(name)

            if match

                @classVersion = match[:version].to_i
                @className = match[:name]
                @classID = match[:classID]                

            else

                match = NAME_REGEX.match(name)

                if match

                    @classVersion = 0
                    @className = match[:name]
                    @classID = match[:classID]

                else

                    raise "class name '#{name}' must be meet requirements"

                end

            end

            if @classVersion > CLASS_VERSION_MAX; raise ArgumentError.new "classVersion must be an Integer in the range (0..#{CLASS_VERSION_MAX})" end

            @methodByName = {}
            @methodByIndex = {}

        end

    
        # Attach a text description to the class
        #
        # @param description [String] description of class
        # @return [String]
        def self.classDescription(description)

            @description = description

        end

        # @return [String] Class Name
        def self.className; return @className end

        # @return [Integer] Class Identifier
        def self.classID; return @classID end

        # @return [Integer] Class Version
        def self.classVersion; return @classVersion end

        # @param methodName [String] optional methodName
        #
        # @return [Hash] table of methods indexed by method name
        # @return [Hash] method description if methodName specified
        def self.methodByName(methodName=nil)

            if methodName                
                @methodByName[methodName]
            else
                @methodByName
            end

        end
        
        # @return [Hash] table of methods indexed by method id
        def self.methodByIndex(methodIndex=nil)

            if methodIndex                
                @methodByName[methodIndex]
            else
                @methodByIndex
            end

        end
                
        # @return [Integer] Object Identifier for this object
        attr_reader :objectID

        # @return [Integer] Object Name for this object
        attr_reader :objectName

        # @return [String] Class Name
        def className; self.class.className end

        # @return [Integer] Class Identifier
        def classID; self.class.classID end        

        # @return [Integer] Class Version
        def classVersion; self.class.classVersion end

        # @param objectID [Integer] unique ObjectIdentifier
        # @param objectName [String, nil] optional human readable name for instance
        def initialize(objectID, objectName=nil, &handlers)

            if self.class.is_a? ObjectClass; raise Exception "may only instanciate a subclass of ObjectClass" end
            if self.classID.nil?; raise Exception "This class definition is incomplete (needs a classDefinition)" end

            if !objectID.is_a? Integer or objectID > OBJECT_ID_MAX; raise ArgumentError.new "objectID '#{objectID}' is not [Integer] in range (0..#{OBJECT_ID_MAX})" end
            if !objectName.nil? and !objectName.is_a? String; raise ArgumentError.new "objectName '#{objectName}' is not [String, nil]" end

            @objectID = objectID
            @objectName = objectName
            @method = {}

            instance_eval(&handlers)

            Peer.addObject self
            
        end

        # Call a method
        #
        # @param caller [Server] the calling server instance
        # @param methodIndex [Integer] method identifier
        # @param argument [String] method invocation argument
        #
        # @return adapterResult [Hash]
        #
        # @option adapterResult [Symbol] :adapterResult 
        # @option adapterResult [Symbol] :result
        # @option adapterResult [String] :returnValue
        #
        def callMethod(caller, methodIndex, argument)
    
            if @method[methodIndex]

                if (@method[methodIndex][:role] & caller.role).size > 0

                    @argument = argument

                    begin

                        { :adapterResult => :PR6_ADAPTER_SUCCESS, :result => :PR6_RESULT_SUCCESS, :returnValue => instance_eval(&@method[methodIndex][:handler]) }

                    rescue MethodHandlerResult => ex

                        { :adapterResult => :PR6_ADAPTER_SUCCESS, :result => ex.message.to_sym }

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

            # method definition 
            #
            # @param methodName [String] name of this method
            # @param opts [Hash] method definition options
            #
            # @options opts [Integer] :methodIndex optional explicit Method Identifier
            #
            def self.defineMethod(methodName, opts = {})

                methodIndex = opts[:methodIndex]

                if !methodName.is_a? String; raise ArgumentError.new "methodName '#{methodName}' is not [String]"end
                if !methodIndex.nil? and ( !methodIndex.is_a? Integer or methodIndex > METHOD_INDEX_MAX); raise ArgumentError.new "opts[:methodIndex] '#{methodIndex}' is not [Integer] in range (0..#{METHOD_INDEX_MAX})" end
                
                if methodIndex.nil?

                    if @methodByIndex.size == 0; methodIndex = 0 else methodIndex = @methodByIndex.keys.last + 1 end

                    if methodIndex > METHOD_INDEX_MAX; raise Exception "Implicitly allocated methodIndex is exhausted at method definition '#{methodName}'" end

                else

                    if !methodByIndex[methodIndex].nil?; raise Exception "opts[:methodIndex] `#{methodIndex}` is already allocated to method `#{@methodByIndex[methodIndex][:methodName]}`" end

                end

                if @methodByName[methodName]; raise "Method '#{methodName}' is already defined" end

                @methodByName[methodName] = {
                    :methodName => methodName,
                    :methodIndex => methodIndex,
                    :description => opts[:description],
                    :argument => opts[:argument],
                    :returnValue => opts[:returnValue]
                }

                @methodByIndex[methodIndex] = @methodByName[methodName]

            end

            
        ################################################################
        private


            # @param methodName [String] name of the method as defined in class
            # @param role [Array<Symbol>] array of roles permitted to invoke this method
            # @param block [Block] method implementation
            def defineMethodHandler(methodName, role, &handler)

                if !methodName.is_a? String; raise ArgumentError.new "methodName must be a String" end
                if !role.is_a? Array; raise ArgumentError.new "role must be an Array of zero or more Symbols" end

                methodDef = self.class.methodByName[methodName]

                if methodDef

                    if @method[methodDef[:methodIndex]]

                        raise "`#{methodName}` handler is already defined for this object"

                    else

                        @method[methodDef[:methodIndex]] = {
                            :handler => handler,
                            :role => role
                        }

                    end

                else

                    raise "methodName \'#{methodName}\' is not defined in class `#{@objectClass.className}`"

                end
                
            end
            
            # Called by #methodHandler to halt execution for reason of argument format
            def haltForArgument; raise MethodHandlerResult.new :PR6_RESULT_ARGUMENT end

            # Called by #methodHandler to halt execution for unspecified permament failure condition
            def haltForPermanent; raise MethodHandlerResult.new :PR6_RESULT_PERMANENT end

            # Called by #methodHandler to halt execution for unspecified temporary failure condition
            def haltForTemporary; raise MethodHandlerResult.new :PR6_RESULT_TEMPORARY end

    end

end
