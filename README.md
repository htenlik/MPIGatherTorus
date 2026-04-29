# MPI_Gather on 2D Torus - OMNeT++ Simulation

This project simulates the MPI_Gather collective communication operation on a 2D Torus topology using OMNeT++.

## Project Topic

- Selected algorithm: MPI_Gather
- Selected topology: 2D Torus
- Simulator: OMNeT++

## Implemented Approaches

### 1. Naive Gather

Every non-root node sends its data directly toward the root using shortest-path routing on the 2D Torus.

### 2. Optimized / Topology-aware Gather

The optimized method performs gather in two phases:

1. Each row gathers data at a row leader.
2. Row leaders send aggregated packets to the root.

This reduces total hop transmissions and root load.

## Simulation Configurations

The following configurations are available in `omnetpp.ini`:

- Naive4x4
- Optimized4x4
- Naive8x8
- Optimized8x8
- Naive16x16
- Optimized16x16

## Results Summary

| Topology | Algorithm | Nodes | Completion Time (s) | Total Hop Transmissions | Average Latency (s) | Root Received Packets | Root Received Payloads |
|----------|-----------|-------|---------------------|--------------------------|---------------------|------------------------|-------------------------|
| 4x4      | Naive     | 16    | 1.0197344           | 32                       | 0.00402709          | 15                     | 15                      |
| 4x4      | Optimized | 16    | 1.025192            | 20                       | 0.004564            | 6                      | 15                      |
| 8x8      | Naive     | 64    | 1.0698528           | 256                      | 0.00816543          | 63                     | 63                      |
| 8x8      | Optimized | 64    | 1.093152            | 144                      | 0.0133291           | 14                     | 63                      |
| 16x16    | Naive     | 256   | 1.2660112           | 2048                     | 0.0162183           | 255                    | 255                     |
| 16x16    | Optimized | 256   | 1.3551616           | 1088                     | 0.0444431           | 30                     | 255                     |

## Conclusion

The optimized gather algorithm reduces total hop transmissions and decreases the number of packets received by the root. However, it slightly increases completion time because row leaders wait for row-level aggregation before sending aggregated packets to the root.
