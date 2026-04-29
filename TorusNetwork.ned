network TorusNetwork
{
    parameters:
        int rows = default(4);
        int cols = default(4);
        int rootRank = default(0);
        string algorithm = default("naive");
        double startTime @unit(s) = default(1s);
        int payloadBytes = default(1024);

        double linkDelay @unit(s) = default(1ms);
        double linkDatarate @unit(bps) = default(10Mbps);

    types:
        channel TorusChannel extends ned.DatarateChannel
        {
        }

    submodules:
        node[rows * cols]: Node {
            parameters:
                rank = index;
                rows = parent.rows;
                cols = parent.cols;
                rootRank = parent.rootRank;
                algorithm = parent.algorithm;
                startTime = parent.startTime;
                payloadBytes = parent.payloadBytes;
        }

    connections:
        for r=0..rows-1, for c=0..cols-1 {
            node[r * cols + c].outEast --> TorusChannel { delay = parent.linkDelay; datarate = parent.linkDatarate; } --> node[r * cols + ((c + 1) % cols)].inWest;
            node[r * cols + c].outWest --> TorusChannel { delay = parent.linkDelay; datarate = parent.linkDatarate; } --> node[r * cols + ((c - 1 + cols) % cols)].inEast;

            node[r * cols + c].outSouth --> TorusChannel { delay = parent.linkDelay; datarate = parent.linkDatarate; } --> node[((r + 1) % rows) * cols + c].inNorth;
            node[r * cols + c].outNorth --> TorusChannel { delay = parent.linkDelay; datarate = parent.linkDatarate; } --> node[((r - 1 + rows) % rows) * cols + c].inSouth;
        }
}