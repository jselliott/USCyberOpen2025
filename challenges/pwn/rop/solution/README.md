# Multi Target

## Solution

The challenge binary is a simple direct buffer overflow onto the stack allowing for a rop chain to occur. The challenge comes from the wording within the description and how the runner works. This is that the given payload must work with 5 different libc targets at once. Luckily the binaries are all ran through qemu, this gives us a few aspects in common. The major difference is that the ld binaries of each of the targets are all loaded at the same address. 

Let's take an aside for a moment to come up with a plan. With the design of this challenge we can't create a rop chain that utilizes a leak and everything needs to be self satisfiable. With the use of qemu this is slightly easier as the ASLR aspects are disabled and the libraries are loaded at knowable addresses. So we need to make a ROP chain that can work for 5 packages within one payload.

To start we need to find a starting gadget, while ASLR may be somewhat nerfed, each package appears to load the libc at a different address. The only two locations of similar usage are within the loader and the binary itself. The binary itself does not have any gadgets that seem to be of use. This narrows us down to the loader.

Running ropper or ROPgadget on the binary and comparing the 5 of them may lead to nothing. To find a working gadget I set up a fuzzer to find any gadgets that successfully return under pc control. Luckily we found a somewhat perfect gadget at offset 126798 within all ld's. 

At this gadget we not only have pc control, we have 4 of the 5 gadgets at different stack offsets. Implying we can now start to interleave 5 rop chains within each other while completing each other's own goal. The last two are much easier to find overlapping gadgets wtihin the ld or libc to offset each other as well. From this we just create 5 rop chains to gain control. I opted to accomplish this through a memprotect to shellcode rop chain combo.
