require 'logger'

module Wrangle

    class Log

        @log

        def self.output=(log)
            @log = Logger::new(log)
            
        end
        def self.level=(level)
            @log.level = level
        end

        def self.debug(s)
            if @log
                @log.debug(s)
            end
        end
        def self.info(s)
            if @log
                @log.info(s)
            end
        end
        
    end

end
