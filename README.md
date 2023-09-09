# Network-Routing-Protocols
The aim of this project is to familiarize with routing algorithms in the network. In this project, we will implement **distance-vector** and **link-state** algorithms and compare them.


## Distance-Vector Routing Protocols (DVRP)
Distance-vector routing protocols rely on the `Bellman-Ford algorithm` to determine the best path between routers in a network. In this type of protocol, each router only has information about the routers it is directly connected to and advertises the **distance value** (DV) of its path to other routers. By exchanging this information, each router can build a routing table to determine the best path to each destination. The routers continue to exchange and update their routing tables until the information converges to stable values.


## Link-State Routing Protocols (LSRP)
In link-state routing protocols, each router possesses information about the complete network topology. Each router then independently calculates the best next hop from it for every possible destination in the network using local information of the topology. The collection of best-next-hops forms the routing table.

This contrasts with distance-vector routing protocols, which work by having each node share its routing table with its neighbors. In a link-state protocol, the only information passed between the nodes is the information used to construct the connectivity maps. 

<br>