#include "Node.h"

Define_Module(Node);

long Node::globalHopTransmissions = 0;

void Node::initialize()
{
    rank = par("rank");
    rows = par("rows");
    cols = par("cols");
    rootRank = par("rootRank");
    algorithm = par("algorithm").stdstringValue();
    payloadBytes = par("payloadBytes");

    numNodes = rows * cols;

    if (rank == 0) {
        globalHopTransmissions = 0;
    }

    for (int i = 0; i < 4; i++) {
        std::string eventName = "endTx" + std::to_string(i);
        endTransmissionEvents[i] = new cMessage(eventName.c_str());
        endTransmissionEvents[i]->setKind(i);
    }

    if (algorithm == "optimized") {
        if (rank == getRowLeaderRank(getRow(rank)) && rank != rootRank) {
            collectedPayloadsInRow = 1;
        }
    }

    EV << "Node started. rank=" << rank
       << " row=" << getRow(rank)
       << " col=" << getCol(rank)
       << " algorithm=" << algorithm << "\n";

    if (rank != rootRank) {
        cMessage *startMessage = new cMessage("startGather");
        startMessage->setKind(-1);

        simtime_t start = par("startTime").doubleValue();
        scheduleAt(start + rank * 0.001, startMessage);
    }
}

void Node::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg->getKind() == -1) {
            delete msg;
            createAndSendOwnData();
        }
        else {
            int gateIndex = msg->getKind();
            startSendingFromQueue(gateIndex);
        }

        return;
    }

    GatherMessage *packet = check_and_cast<GatherMessage *>(msg);

    if (rank == packet->getFinalDestRank()) {
        handlePacketAtDestination(packet);
    }
    else {
        forwardedPackets++;
        routePacket(packet);
    }
}

void Node::createAndSendOwnData()
{
    if (algorithm == "optimized") {
        createAndSendOwnDataOptimized();
    }
    else {
        createAndSendOwnDataNaive();
    }
}

void Node::createAndSendOwnDataNaive()
{
    GatherMessage *packet = new GatherMessage("gatherData");

    packet->setSourceRank(rank);
    packet->setFinalDestRank(rootRank);
    packet->setPayloadCount(1);
    packet->setHopCount(0);
    packet->setByteLength(payloadBytes);

    sentPackets++;

    EV << "Node " << rank << " sends its data directly to root " << rootRank << "\n";

    routePacket(packet);
}

void Node::createAndSendOwnDataOptimized()
{
    int myRow = getRow(rank);
    int rowLeaderRank = getRowLeaderRank(myRow);

    if (rank == rowLeaderRank) {
        EV << "Row leader node " << rank << " keeps its own data in row aggregate\n";
        trySendRowAggregate();
        return;
    }

    GatherMessage *packet = new GatherMessage("rowGatherData");

    packet->setSourceRank(rank);
    packet->setFinalDestRank(rowLeaderRank);
    packet->setPayloadCount(1);
    packet->setHopCount(0);
    packet->setByteLength(payloadBytes);

    sentPackets++;

    EV << "Node " << rank
       << " sends its data to row leader " << rowLeaderRank
       << "\n";

    routePacket(packet);
}

void Node::handlePacketAtDestination(GatherMessage *packet)
{
    if (rank == rootRank) {
        handlePacketAtRoot(packet);
    }
    else {
        handlePacketAtRowLeader(packet);
    }
}

void Node::handlePacketAtRoot(GatherMessage *packet)
{
    receivedPacketsAtRoot++;
    receivedPayloadsAtRoot += packet->getPayloadCount();
    totalLatencyAtRoot += simTime() - packet->getCreationTime();

    EV << "ROOT received packet from source="
       << packet->getSourceRank()
       << " payloadCount=" << packet->getPayloadCount()
       << " hopCount=" << packet->getHopCount()
       << " time=" << simTime()
       << "\n";

    delete packet;

    if (receivedPayloadsAtRoot == numNodes - 1) {
        double averageLatency = totalLatencyAtRoot.dbl() / receivedPacketsAtRoot;

        recordScalar("gatherCompletionTime", simTime());
        recordScalar("totalHopTransmissions", globalHopTransmissions);
        recordScalar("averagePacketLatencyAtRoot", averageLatency);
        recordScalar("rootReceivedPackets", receivedPacketsAtRoot);
        recordScalar("rootReceivedPayloads", receivedPayloadsAtRoot);

        EV << "\n";
        EV << "===== MPI_Gather simulation finished =====\n";
        EV << "Algorithm: " << algorithm << "\n";
        EV << "Nodes: " << numNodes << "\n";
        EV << "Root rank: " << rootRank << "\n";
        EV << "Gather completion time: " << simTime() << "\n";
        EV << "Total hop transmissions: " << globalHopTransmissions << "\n";
        EV << "Average packet latency at root: " << averageLatency << "\n";
        EV << "Root received packets: " << receivedPacketsAtRoot << "\n";
        EV << "Root received payloads: " << receivedPayloadsAtRoot << "\n";
        EV << "==========================================\n";

        endSimulation();
    }
}

void Node::handlePacketAtRowLeader(GatherMessage *packet)
{
    collectedPayloadsInRow += packet->getPayloadCount();

    EV << "Row leader node " << rank
       << " collected packet from source=" << packet->getSourceRank()
       << " collectedPayloadsInRow=" << collectedPayloadsInRow
       << "\n";

    delete packet;

    trySendRowAggregate();
}

void Node::trySendRowAggregate()
{
    if (algorithm != "optimized") {
        return;
    }

    if (rowAggregateSent) {
        return;
    }

    if (rank == rootRank) {
        return;
    }

    if (rank != getRowLeaderRank(getRow(rank))) {
        return;
    }

    if (collectedPayloadsInRow < cols) {
        return;
    }

    GatherMessage *aggregatePacket = new GatherMessage("rowAggregate");

    aggregatePacket->setSourceRank(rank);
    aggregatePacket->setFinalDestRank(rootRank);
    aggregatePacket->setPayloadCount(collectedPayloadsInRow);
    aggregatePacket->setHopCount(0);
    aggregatePacket->setByteLength(payloadBytes * collectedPayloadsInRow);

    rowAggregateSent = true;
    sentPackets++;

    EV << "Row leader node " << rank
       << " sends aggregated row data to root. payloadCount="
       << collectedPayloadsInRow << "\n";

    routePacket(aggregatePacket);
}

void Node::routePacket(GatherMessage *packet)
{
    const char *outputGate = chooseOutputGate(packet->getFinalDestRank());

    packet->setHopCount(packet->getHopCount() + 1);
    globalHopTransmissions++;

    EV << "Node " << rank
       << " forwards packet from source=" << packet->getSourceRank()
       << " to finalDest=" << packet->getFinalDestRank()
       << " using gate=" << outputGate
       << " hopCount=" << packet->getHopCount()
       << "\n";

    sendOrQueue(packet, outputGate);
}

void Node::sendOrQueue(GatherMessage *packet, const char *outputGate)
{
    int gateIndex = getGateIndex(outputGate);

    gateQueues[gateIndex].push(packet);

    if (!endTransmissionEvents[gateIndex]->isScheduled()) {
        startSendingFromQueue(gateIndex);
    }
}

void Node::startSendingFromQueue(int gateIndex)
{
    if (gateQueues[gateIndex].empty()) {
        return;
    }

    const char *outputGate = outputGateNames[gateIndex];

    cGate *gatePointer = gate(outputGate);
    cDatarateChannel *channelPointer = check_and_cast<cDatarateChannel *>(gatePointer->getTransmissionChannel());

    simtime_t finishTime = channelPointer->getTransmissionFinishTime();

    if (finishTime > simTime()) {
        if (!endTransmissionEvents[gateIndex]->isScheduled()) {
            scheduleAt(finishTime, endTransmissionEvents[gateIndex]);
        }
        return;
    }

    GatherMessage *packet = gateQueues[gateIndex].front();
    gateQueues[gateIndex].pop();

    send(packet, outputGate);

    if (!gateQueues[gateIndex].empty()) {
        simtime_t nextTime = channelPointer->getTransmissionFinishTime();
        scheduleAt(nextTime, endTransmissionEvents[gateIndex]);
    }
}

const char *Node::chooseOutputGate(int destinationRank) const
{
    int myRow = getRow(rank);
    int myCol = getCol(rank);

    int destRow = getRow(destinationRank);
    int destCol = getCol(destinationRank);

    int eastDistance = (destCol - myCol + cols) % cols;
    int westDistance = (myCol - destCol + cols) % cols;

    if (myCol != destCol) {
        if (eastDistance <= westDistance) {
            return "outEast";
        }
        else {
            return "outWest";
        }
    }

    int southDistance = (destRow - myRow + rows) % rows;
    int northDistance = (myRow - destRow + rows) % rows;

    if (myRow != destRow) {
        if (southDistance <= northDistance) {
            return "outSouth";
        }
        else {
            return "outNorth";
        }
    }

    return "outEast";
}

int Node::getGateIndex(const char *gateName) const
{
    std::string name = gateName;

    if (name == "outNorth") {
        return 0;
    }
    else if (name == "outSouth") {
        return 1;
    }
    else if (name == "outEast") {
        return 2;
    }
    else {
        return 3;
    }
}

int Node::getRow(int rankValue) const
{
    return rankValue / cols;
}

int Node::getCol(int rankValue) const
{
    return rankValue % cols;
}

int Node::getRank(int row, int col) const
{
    return row * cols + col;
}

int Node::getRootRow() const
{
    return getRow(rootRank);
}

int Node::getRootCol() const
{
    return getCol(rootRank);
}

int Node::getRowLeaderRank(int row) const
{
    return getRank(row, getRootCol());
}

void Node::finish()
{
    recordScalar("sentPackets", sentPackets);
    recordScalar("forwardedPackets", forwardedPackets);

    for (int i = 0; i < 4; i++) {
        cancelAndDelete(endTransmissionEvents[i]);

        while (!gateQueues[i].empty()) {
            GatherMessage *packet = gateQueues[i].front();
            gateQueues[i].pop();
            delete packet;
        }
    }
}
