require 'wrangle/method_request'

module Wrangle

    class MethodResponse

        # @param req [MethodRequest] the request this response answers
        # @param result [Symbol] result of invocation
        # @param returnValue [nil, String] return value from successful invocation
        #
        def initialize(req, result, returnValue=nil)

            @result = result

            if !req.is_a? MethodRequest
                raise ArgumentError
            end
            if !PR6_CLIENT_RESULT.include? result
                raise ArgumentError
            end

            @req = req
            @result = result

            if result == :PR6_RESULT_SUCCESS

                @returnValue = returnValue.to_s
                
            end
            
        end

        # @return [MethodRequest] request this response answers
        attr_reader :req

        # @return [Symbol] result of invocation
        attr_reader :result

        # @return [nil,String] return value
        attr_reader :returnValue

    end

end
