/*
 * Node.h
 *
 *  Created on: Apr 29, 2026
 *      Author: huseyintenlik
 */
#ifndef __MPIGATHERTORUS_NODE_H_
#define __MPIGATHERTORUS_NODE_H_

#include <omnetpp.h>
#include <queue>
#include <string>
#include "GatherMessage_m.h"

using namespace omnetpp;

class Node : public cSimpleModule
{
  private:
    int rank;
    int rows;
    int cols;
    int rootRank;
    int payloadBytes;
    int numNodes;

    std::string algorithm;

    int receivedPacketsAtRoot = 0;
    int receivedPayloadsAtRoot = 0;
    simtime_t totalLatencyAtRoot = SIMTIME_ZERO;

    int collectedPayloadsInRow = 0;
    bool rowAggregateSent = false;

    int sentPackets = 0;
    int forwardedPackets = 0;

    static long globalHopTransmissions;

    std::queue<GatherMessage *> gateQueues[4];
    cMessage *endTransmissionEvents[4] = {nullptr, nullptr, nullptr, nullptr};

    const char *outputGateNames[4] = {
        "outNorth",
        "outSouth",
        "outEast",
        "outWest"
    };

    int getRow(int rankValue) const;
    int getCol(int rankValue) const;
    int getRank(int row, int col) const;

    int getRootRow() const;
    int getRootCol() const;
    int getRowLeaderRank(int row) const;

    int getGateIndex(const char *gateName) const;

    void createAndSendOwnData();
    void createAndSendOwnDataNaive();
    void createAndSendOwnDataOptimized();

    void handlePacketAtDestination(GatherMessage *packet);
    void handlePacketAtRoot(GatherMessage *packet);
    void handlePacketAtRowLeader(GatherMessage *packet);

    void trySendRowAggregate();

    void routePacket(GatherMessage *packet);
    const char *chooseOutputGate(int destinationRank) const;

    void sendOrQueue(GatherMessage *packet, const char *outputGate);
    void startSendingFromQueue(int gateIndex);

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif
