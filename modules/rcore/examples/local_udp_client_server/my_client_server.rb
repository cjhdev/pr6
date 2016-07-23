require 'wrangle'
require 'sqlite3'
require 'pp'

Thread.abort_on_exception = true

Wrangle::Log.output = STDERR
Wrangle::Log.level = Logger::INFO

# some constants referenced below
CLIENT_ENTITY_ID="00-00-00-00-00-00-00-01"
SERVER_ENTITY_ID="00-00-00-00-00-00-00-FF"
OBJECT_ID = 0

# setup an in-memory database to store association data
Sequel.extension :migration
db = Sequel.sqlite
Sequel::Migrator.run(db, "#{Dir.pwd}/stage/db/migrations")
Wrangle::AssociationRecord.db = db

# add an association record to the client to associate it with the server
Wrangle::AssociationRecord.create(
    CLIENT_ENTITY_ID,
    CLIENT_ENTITY_ID,
    SERVER_ENTITY_ID,
)

# add an association record to server to associate it with the client
Wrangle::AssociationRecord.create(
    SERVER_ENTITY_ID,
    SERVER_ENTITY_ID,
    CLIENT_ENTITY_ID,
    assignedRole: [:public]
)

# define a class called echo with one method of the same name
class EchoCID0000 < Wrangle::ObjectClass
    defineMethod "echo"    
end

# insert and instance of echo into an object list
serverObjectList = []
serverObjectList << EchoCID0000.new(OBJECT_ID) do
    defineMethodHandler("echo", [:public]) do |arg|
        
        arg
    end
end

# init the server node
server = Wrangle::UDPNode.new(SERVER_ENTITY_ID, objectList: serverObjectList)

# init the client node
client = Wrangle::UDPNode.new(CLIENT_ENTITY_ID)

puts "server operating on port #{server.port}"
puts "client operating on port #{client.port}"

# every second, client will send the following request to the server
loop do

    result = client.request(SERVER_ENTITY_ID, ip:"127.0.0.1", port: server.port) do
        request(OBJECT_ID,0,"hello")
        request(OBJECT_ID,0,"hello")
        request(OBJECT_ID,0,"hello")
        request(OBJECT_ID,0,"hello")
        request(OBJECT_ID,0,"hello")
    end

    pp result

    sleep(2)

end
