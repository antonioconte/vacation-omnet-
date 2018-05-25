//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "PassiveQueue.h"
#include "Job.h"
#include "IServer.h"
#include "Server.h"

namespace queueing {

    Define_Module(PassiveQueue);

    PassiveQueue::PassiveQueue(){}
    PassiveQueue::~PassiveQueue(){}

    void PassiveQueue::initialize(){
        boringJobSignal = registerSignal("jobExpired");
        executedJobSignal = registerSignal("jobExecuted");
        capacity = par("capacity");
        queue.setName("queue");
        vacation = false;
        occuped = false;
    }

    void PassiveQueue::handleMessage(cMessage *msg){
        Job *job = check_and_cast<Job *>(msg);
        job->setTimestamp();
        // check for container capacity
        if (capacity >= 0 && queue.getLength() >= capacity) {
            EV << "Queue full! Job dropped.\n";
            if (hasGUI())
                bubble("Dropped!");
            delete msg;
            return;
        }

        // Check sul time expire di ogni job nella coda:
        // se il getLimitTime() > simTime() allora rimuovo il job dalla coda
        for(int i=0; i<queue.getLength(); i++){
            EV << "\nCHECK PER IL LIMIT JOB TIME";
            Job *currentJob = (Job *)queue.get(i);
            if( currentJob->getTimeLimit() < simTime()){
                EV << "\nJOB BORING ##############";
                queue.remove(currentJob);
                job->setQueueCount(job->getQueueCount() - 1);
                sendToSink(currentJob);
            }
        }

        // gate: 0 --> SERVER  // gate: 1 --> SINK
        cGate *out = gate("out", 0);
        occuped = !check_and_cast<Server *>(out->getPathEndGate()->getOwnerModule())->isIdle();
        vacation = check_and_cast<Server *>(out->getPathEndGate()->getOwnerModule())->inVacation();

        EV << "\nSERVER VACATION ????? -> " << (vacation ? "SI" : "NO");
        if (occuped && !vacation) { // metto in coda il lavoro se il server è occupato
            queue.insert(job);
            job->setQueueCount(job->getQueueCount() + 1);
        } else if(!occuped){  // non considero il gate del pozzo
            //ALLORA IL SERVER è IN VACATION O IDLE
            if(!vacation) sendJob(job); //idle
            else{  //VACATION
                queue.insert(job);
                job->setQueueCount(job->getQueueCount() + 1);
            }
        }
       }

    void PassiveQueue::refreshDisplay() const{
        // change the icon color
        getDisplayString().setTagArg("i", 1, queue.isEmpty() ? "" : "cyan");
    }

    int PassiveQueue::length(){
        return queue.getLength();
    }

    //Chiamata dal server per la richiesta di un nuovo lavoro
    void PassiveQueue::request(int gateIndex){
        Enter_Method("request()!");
        Job *job = (Job *)queue.pop();
        job->setQueueCount(job->getQueueCount()+1);
        simtime_t d = simTime() - job->getTimestamp();
        job->setTotalQueueingTime(job->getTotalQueueingTime() + d);
        sendJob(job);
    }


    void PassiveQueue::sendJob(Job *job){
        cGate *out = gate("out", 0);
        send(job, out);
        emit(executedJobSignal, 1);              // Aggiorno il numero di JobEseguiti
        check_and_cast<IServer *>(out->getPathEndGate()->getOwnerModule())->allocate();
    }

    void PassiveQueue::sendToSink(Job *job){
        emit(boringJobSignal, 1);
        cGate *out = gate("out", 1);
        send(job, out);
    }
}; //namespace

