# Install Build Tools
    apt install qemu-user-static gcc-arm-linux-gnueabi binutils-arm-linux-gnueabi libc6-armel-cross


## Build the Challenge
	arm-linux-gnueabi-g++ challenge.cpp -o chal

## Run the Binary
    \# If necessary, let qemu know where the ARM libraries are
	qemu-armel-static -L /usr/arm-linux-gnueabi/ chal 


## Build the Dam

* The partial dam can be built by modifying the solve [script](dam.py) to print the first 64 steps.
We just need to include enough steps that all necessary state transitions have been seen.

    ./dam.py 5

* View the dam with line numbers for the first 64 state changes
    nl -v 0 dist/dam.partial


## Generate the Flag

Choose a flag and update the string in the 'flag' variable of this modified solution [script](flag_gen.py) to get a list of step numbers which have a count of ones correspond to the ASCII value of each character.

    ./flag_gen.py 5
