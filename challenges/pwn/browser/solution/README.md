# Quicker

This challenge introduces a vulnerability into the quickjs javascript engine. The challengers are presented a git diff of the modified files and can see 2 main differences. The first is that import are disabled, the second is that a new function is introduced as 'swap'. This method is introduced into the prototype of arrays, the code is clearly vulnerable with lacking bounds checking for the swapped data. Using this we can get an out of bounds read and write capability.

The interesting part of this challenge is not that there is no heap isolate, in fact it is the oposite, this javascript challenge is entirly in the heap. By this I mean, every line written within the file changes your heap layout and the heap appears fairly inconsistent. A major portion of this challenge is to find a semit reliable path through this noise.

As an out of bounds read and write is capable, it should be possible to target any pointer or metadata of objects within the heap. During testing I identified that one of the negative offsets contained a function pointer that was somewhat reliable and the input is controllable as well. 

So the first goal is to gain a leak, this can be accomplished by walking through the heap and looking for an egg that lines up with a standard libc address, this usually lines up with an unsorted bin or a large bin pointer. Having the libc leak and the function overwrite allowed for a rop chain to be set into memory.

This chain could be further refined but the approach I took required the chain to only have control of every other 8 bytes, (big int meta data was in between). With careful gadget choosing and placement along with a bit of stack spraying for register alignment, allowed for bin/sh to be loaded and a call to system to occur.

### Deployment Instructions

Run src/deploy-docker.sh to build and deploy the app

