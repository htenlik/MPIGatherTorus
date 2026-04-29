//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//


simple Node
{
    parameters:
        int rank;
        int rows;
        int cols;
        int rootRank = default(0);
        string algorithm = default("naive");
        double startTime @unit(s) = default(1s);
        int payloadBytes = default(1024);

        @display("i=device/pc");

    gates:
        input inNorth;
        input inSouth;
        input inEast;
        input inWest;

        output outNorth;
        output outSouth;
        output outEast;
        output outWest;
}