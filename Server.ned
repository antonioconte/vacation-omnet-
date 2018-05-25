//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


//
// Queue server. It serves multiple input queues (PassiveQueue), using a preset
// algorithm. Inputs must be connected to Passive Queues (PassiveQueue)
//
simple Server
{
    parameters:
        @group(Queueing);
        @display("i=block/server");
        @signal[busy](type="bool");
        @signal[vacation](type="long");
        @statistic[vacation](title="Numero di Vacation";record=vector?,count;interpolationmode=none);
        @statistic[busy](title="server busy state";record=vector?,timeavg;interpolationmode=sample-hold);
	    
	    @signal[lengthBusy](type="long");
        @signal[lengthVacation](type="long");
        @statistic[lengthVacation](title="queue length in Vacation State";record=vector,timeavg,max;interpolationmode=sample-hold);
        @statistic[lengthBusy](title="queue length in Busy\IDLE State";record=vector,timeavg,max;interpolationmode=sample-hold);
		
		int numVacation = default(-1);
        string fetchingAlgorithm @enum("priority","random","roundRobin","longestQueue") = default("priority");
        // how the next job will be choosen from the attached queues
        volatile double serviceTime @unit(s); // service time of a job
    	double vacationTime @unit(s);
        gates:
        input in[];
        output out;
}


