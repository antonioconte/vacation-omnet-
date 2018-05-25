network Network
{
    submodules:
        passiveQueue: PassiveQueue {
            @display("p=171,103");
        }
        source: Source {
            @display("p=58,47");
        }
        server: Server {
            @display("p=291,47");
        }
        sink: Sink {
            @display("p=301,140");
        }
    connections:
        source.out --> passiveQueue.in++;
        passiveQueue.out++ --> server.in++;
        server.out --> sink.in++;
        passiveQueue.out++ --> sink.in++;
}

