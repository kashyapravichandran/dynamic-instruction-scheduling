# Dynamic Instruction Scheduling

The program tries to simulate the working of a out-of-order super scalar processor with bandwidth 'N' and Issue Queue of size 'S'. The program uses a fake Re-Order Buffer which makes sure that the instructions are retired in order. There are 6 stages in this processor, namely: 

- Fetch 
- Decode
- Schedule 
- Execute / Memory  
- Write Back 
- Retire 

The processor can have a total of 2S instruction in its fetch and decode stage. The processor stalls fetching new instruction when the decode queue is full. There are 3 types of instruction, this processor tries to simluate. Type 1 tries to simulate instruction which takes 1 clock cycle to finish executing in the EX stage. Type 2 are those instruction which consumes more than 1 clock cycle, in this simulation type 2 takes 2 clock cycle. Type 3 instructions are memory based instruction which takes 5 cycles when the load/store hits in the L1 cache, 10 cycle when it hits in the L2 cache and 20 when it misses L2. The detailed explanation for this type of a processor is given in the spec document. 

## Usage

In a Linus machine, you can run a make command to get a executable. Run the command given below to get the timing diagram for the given trace file. 

Trace file entry : <PC> <Type> <Destination Register> <Source 1> <Source 2> <Memory Address>
where,  
  PC is the program counter
 
 When a particular register is not needed, we denote it using a -1. When the instruction is a non memory based instruction, the memory address is 0. 
 
 The output from the simulator is as follow: 
 
 <Sequence Number> fu{<Type>} src{<Source 1>,<Source 2>} dst{<Destination Register>} IF{<begin cycle>,<duration>} ID{...} IS{...} EX{...} WB{...}
  
 The above given format is followed so that the output obtained can be passed to a scope tool that tries to display the timing diagram in a format shown below for say 3 instruction A,B, and C. Let says they don't have any type of hazards. 
 
 Inst    Cycle 1   Cycle 2   Cycle 3   Cycle 4   Cycle 5   Cycle 6   Cycle 7
   A       IF        ID        IS        EX        WB
   B       IF        ID        IS        EX        WB
   C                 IF        ID        IS        EX        WB
 
 Let us assume that the bandwidth of the processor is 2. 
 
 To run the simulator, use the following command: 
 
          ./sim.out <S> <N> <Block Size> <L1_Size> <L1_Associativity> <L2_Size> <L2_Associativity> <Trace File>
          
          
The scope tool was developed by facult and students at NC State University, so I am not at a liberty to publish it on my github repo. However, the output observed is pretty understable on its own. 
