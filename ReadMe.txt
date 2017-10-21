========================================================================
    STATIC LIBRARY : PearlLib Project Overview
========================================================================

Mother of Toiletseat
Capture / Processing library

A 2 channel multi threaded processor framework.
Uses Capture devices as inputs, codecs as transforms, and provides an interface to receive the end results.

1) A processor is created. 
2) A graph is created and the ISampleInput of the processor is given to the graph ISampleGrabber callback handlers.
3) Codecs are wrapped by an ISampleTransform impl and given to the processor.
4) After starting, the samples are queued up from the capture graph.
5) Two threads begin compressing the two channels, and passing the results to an output queue.
6) A third thread pulls the compressed data packets from the output ques, and passes them to the user ISampleInput implementation. 
7) The sample packet is deleted.
8) When destroyed, the capture graph and threads are stopped, and all queued packets are deleted.


 __________        
|          |
| capture  |
|__________|
      |
 ISampleInput 
 _____|_____        __________________
|           |      |                  |
| Processor |------| ISampleTransform |
|___________|      |__________________|
      | 
 ISampleInput
 _____|_______        
|             |  
|  User impl  |
|_____________|


