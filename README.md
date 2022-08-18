# CPP-ryptography
This is program is a cpp solver to find the minimum cost to find a combination where the diffusion on a feistel network with k blocks is maximum

## CAREFUL 
This is a proof of concept, and was for me the first time is used c++ in a project. Result must not be taken as a solution (results not proved).

## Generate Partitions of chains and cycles  with k pairs of blocks
it is generating all the possible combinations of k pairs of blocks and storing them in a vector of pairs containing vector of chains and cycles storing it in a file : partitions/part-k.txt
```bash
./parts <k : number of pairs >
```

## Generate all skeletons for all partitions of chains and cycles with k pairs of blocks or for a specific list of partitions
it is generating from the file partitions/part-k.txt or partitions/part-id.txt  all the possible skeletons for all the partitions of chains and cycles with k pairs of blocks and storing them in a boost archive : graphGen/graphs-k.txt or graphGen/graphs-id.txt


```bash
./graphs <k : number of pairs> (facultative):<id : file with partitions withs k pairs>
``` 

# Resolve all skeletons within a boost archive of graphs with k pairs of blocks or for a specific list of graphs
it is solving all the skeletons within a boost archive of graphs with k pairs of blocks or for a specific list of graphs within graphGen/graphs-k.txt or graphGen/graphs-id.txt and say if it exists a solution or not. if a solution exists it is soring markdowns with all solutions graphs in the dot language and the minimum cost r. the markdowns are generated with a slicing of a defined number of graphs hardocded in the variable batchSizeDisplay.

the markdowns with solutions are visible in the folder markdowns/k-nodes/solutions-classic/r-cost/ or markdowns/k-nodes/solutions-id/r-cost/ if an id was provided in the command line.

```bash
./solve <k : number of pairs> <r : cost of path> (facultative):<id : archive with skeletons graphs with k pairs>
```