MPI_Gather on 2D Torus - OMNeT++ Simulation

This project simulates MPI_Gather communication on a 2D Torus topology using OMNeT++.

Selected algorithm: MPI_Gather
Selected topology: 2D Torus

Implemented approaches:
1. Naive gather
   - Every non-root node sends its data directly to the root using shortest-path routing.

2. Optimized / topology-aware gather
   - First, each row gathers data at a row leader.
   - Then, row leaders send aggregated data to the root.
   - This reduces total hop transmissions and root load.

Available simulation configurations in omnetpp.ini:
- Naive4x4
- Optimized4x4
- Naive8x8
- Optimized8x8
- Naive16x16
- Optimized16x16

Main files:
- TorusNetwork.ned: Defines the 2D Torus network.
- Node.ned: Defines node parameters and gates.
- src/Node.cc: Implements naive and optimized MPI_Gather behavior.
- src/Node.h: Header file for Node module.
- src/GatherMessage.msg: Defines gather packet fields.
- results.txt: Raw simulation results.
- report_notes.txt: Explanation, result table, analysis, and conclusion.

How to run:
1. Open the project in OMNeT++ IDE.
2. Build the project.
3. Right click omnetpp.ini.
4. Select Run As -> OMNeT++ Simulation.
5. Choose one of the configurations such as Naive4x4 or Optimized4x4.