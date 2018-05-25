//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __QUEUEING_SERVER_H
#define __QUEUEING_SERVER_H

#include "IServer.h"

namespace queueing {

class Job;
class SelectionStrategy;

/**
 * The queue server. It cooperates with several Queues that which queue up
 * the jobs, and send them to Server on request.
 *
 * @see PassiveQueue
 */
class QUEUEING_API Server : public cSimpleModule, public IServer
{
    private:
        simsignal_t queueVacationSignal;
        simsignal_t queueBusySignal;
        simsignal_t busySignal;
        simsignal_t vacationSignal;
        bool allocated;
        bool vacation;
        simtime_t vacationTime;
        SelectionStrategy *selectionStrategy;
        int numVacation;
        int localVacation;
        Job *jobServiced;
        cMessage *endServiceMsg;
        cMessage *endVacationMsg;

    protected:
        virtual void initialize() override;
        virtual int numInitStages() const override {return 2;}
        virtual void handleMessage(cMessage *msg) override;
        virtual void refreshDisplay() const override;
        virtual void finish() override;

    public:
        Server();
        virtual ~Server();
        virtual bool isIdle() override;
        virtual void allocate() override;
        bool inVacation();
        int lengthQueue();
};

}; //namespace

#endif


