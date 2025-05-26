# One Byte to Heap Them All 

## Solution

This challenge is to test a challengers ability to utilize heap corruption techniques and to pivot through FSOP corruption. The challenge in question is againsta modern libc. The challenge itself allows for a user to create a chunk and put data within said chunk, and to delete a chunk. The user is allowed at most 15 chunks, but the twist is that the data read in is base64 decoded. There are verifications to make sure the bytes read in are valid base64 characters and the decoding "seems" to be safe.

The vulnerability is based off of CVE-2018-6789, where the allocation and decoding of the base64 data is incorrectly calculated and operated on before verifying the authenticity of a base64 string. An example would be give a non 6 byte aligned string without '=' character, this results in an incorrect size allocation and a 1 byte out of bounds write. Using this we can corrupt heap metadata to get overlapping chunks.

From this point there are a few different techniques that can be utilized to gain a leak, the approach I took was to heap feng shu to corrupt a large bin pointer to corrupt the _mp structure and modify the tcache max size and count variables. This confuses the heap to think much larger sizes exist within the heap, using this I overwrite an unsorted bin pointer to allocate into IO_stdout and use this to leak the libc. Lastly I use the same tcache corruption to overwrite stdout for an FSOP call to system.
