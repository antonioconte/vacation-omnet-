//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "Server.h"
#include "Job.h"
#include "SelectionStrategies.h"
#include "IPassiveQueue.h"

namespace queueing {

  Define_Module(Server);

  Server::Server() {
    selectionStrategy = nullptr;
    jobServiced = nullptr;
    endServiceMsg = nullptr;
    allocated = false;
  }

  Server::~Server() {
    delete selectionStrategy;
    delete jobServiced;
    cancelAndDelete(endServiceMsg);
  }

  void Server::initialize() {
    queueVacationSignal = registerSignal("lengthVacation");
    queueBusySignal = registerSignal("lengthBusy");
    busySignal = registerSignal("busy");
    vacationSignal = registerSignal("vacation");
    // In fase di inizializzazione il server non è occupato
    emit(busySignal, false);

    emit(queueVacationSignal, 0);
    emit(queueBusySignal, 0);


    vacationTime = par("vacationTime");
    endVacationMsg = new cMessage("end-vacation");
    endServiceMsg = new cMessage("end-service");
    vacation = false;
    numVacation = par("numVacation");
    localVacation = numVacation;
    jobServiced = nullptr;
    allocated = false;
    selectionStrategy = SelectionStrategy::create(par("fetchingAlgorithm"), this, true);
    if (!selectionStrategy)
      throw cRuntimeError("invalid selection strategy");
  }

  void Server::handleMessage(cMessage * msg) {
//    EV << "CODA ------> " << this->lengthQueue();
    if (msg == endServiceMsg) {
        //IL SERVER HA FINITO DI ELABORARE UN JOB
        // AGGIORNO LA STAT SULLA LUNGHEZZA DELLA CODA
        //DOPO LO STATO BUSY
        emit(queueBusySignal, lengthQueue());
      vacation = false;
      cancelEvent(endVacationMsg);
      localVacation = numVacation;
      ASSERT(jobServiced != nullptr);
      ASSERT(allocated);
      simtime_t d = simTime() - endServiceMsg->getSendingTime();
      jobServiced-> setTotalServiceTime(jobServiced-> getTotalServiceTime() + d);
      send(jobServiced, "out");
      jobServiced = nullptr;
      allocated = false;
      emit(busySignal, false);
      int k = selectionStrategy->select();
      if (k >= 0) {
        //SE entro qui allora la coda non è vuota
        cGate *gate = selectionStrategy->selectableGate(k);
        check_and_cast<IPassiveQueue *>(gate->getOwnerModule())->request(gate-> getIndex());
      } else {
        //UPDATE SIGNAL VACATION QUEUE
        emit(busySignal, false);
        emit(vacationSignal, 1);

        emit(queueVacationSignal, lengthQueue());
        EV << "\nQUEUE VUOTA : SERVER IN VACATION T= " << vacationTime << "\n";
        vacation = true;
        scheduleAt(simTime() + vacationTime, endVacationMsg);
      }
    } else if (msg == endVacationMsg) {
      //IL SERVER HA FINITO LA VACATION ALLORA
      //AGGIORNO LA STAT SULLA LUNGHEZZA DELLA CODA DOPO UNA VACATION
      emit(queueVacationSignal, lengthQueue());
      localVacation--;
      int k = selectionStrategy->select();
      if (k >= 0) {         //SE entro qui allora la coda non è vuota
        EV << "\nFINE VACATION --> ARRIVO DI UN JOB";
        localVacation = numVacation;
        vacation = false;
        cancelEvent(endVacationMsg);
        cGate * gate = selectionStrategy->selectableGate(k);
        check_and_cast <IPassiveQueue *>(gate->getOwnerModule())->request(gate->getIndex());
      } else {
        if (localVacation == 0) { //limite vacation raggiunto
          EV << "\nNumero vacation Raggiunto -->  IDLE STATE";
          localVacation = numVacation;
          vacation = false;
          cancelEvent(endVacationMsg);
        } else {
          EV << "\nVacation Rimanenti: " << localVacation;
          vacation = true;
          scheduleAt(simTime() + vacationTime, endVacationMsg);
        }
      }
    } else { //QUI VIENE ELABORATO IL JOB
      if (!allocated)
        error("job arrived, but the sender did not call allocate() previously");
      if (jobServiced)
        throw cRuntimeError("a new job arrived while already servicing one");
      jobServiced = check_and_cast < Job * > (msg);
      simtime_t serviceTime = par("serviceTime");
      scheduleAt(simTime() + serviceTime, endServiceMsg);
      emit(queueBusySignal, lengthQueue());
      emit(busySignal, true); //SEGNALO L'UTILIZZO DEL SERVER
    }
  }

  bool Server::inVacation() {
    return vacation;
  }

  void Server::refreshDisplay() const {
    getDisplayString().setTagArg("i2", 0, jobServiced ? "status/execute" : "");
    getDisplayString().setTagArg("i", 1, vacation ? "yellow" : "");

  }

  void Server::finish() {}

  bool Server::isIdle() {
    return !allocated; // we are idle if nobody has allocated us for processing
  }

  void Server::allocate() {
    allocated = true;
  }

  int Server::lengthQueue(){
      cGate *gate = selectionStrategy->selectableGate(0);
      int l = check_and_cast <IPassiveQueue *>(gate->getOwnerModule())->length();
      return l;
  }

}; //namespace
