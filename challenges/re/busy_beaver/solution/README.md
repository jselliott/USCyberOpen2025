# Solution

Use the first 64 state transitions given in the partial dam to determine the correct configuration of the corrupted Turing Machine.

All states except halt are encountered in the partial dam, which is enough to reconstruct what we need.

Read about how Busy Beaver machines transition from state to state [here](https://catonmat.net/busy-beaver)

## Corrupted Turing Machine

Adapted Busy Beaver machine [7410754](https://bbchallenge.org/7410754)


    "a0" -> "b1r"
    "b0" -> "c1l"
    "c0" -> "a1l"
    "d0" -> "!@#"
    "e0" -> "$%^"
    "a1" -> "&*("
    "b1" -> "c1r"
    "c1" -> "c0r"
    "d1" -> "e0l"
    "e1" -> "):?"

## Corrected Turing Machine

    "a0" -> "b1r"
    "b0" -> "c1l"
    "c0" -> "a1l"
    "d0" -> "h1r"
    "e0" -> "b0r"
    "a1" -> "d0l"
    "b1" -> "c1r"
    "c1" -> "c0r"
    "d1" -> "e0l"
    "e1" -> "d1l"

You can now patch the binary to make it run all the necessary states, or write an external script

**Remember:** The binary is set to only print the tape when there is a state change, so you must make sure you print ALL states to track the indexes properly.

## Getting the Flag

You can now print out all the states of this Busy Beaver to complete the dam.  The phrase shouted by Belinda Beaver corresponds to the states of interest. Since the Busy Beaver problem formally asks about how many ones can be printed on the tape, that count of ones is what we seek to decode the flag.

For example, her first word is 6719.  So we need to find the number of ones printed out in the 6719th iteration.  There are 83 ones at this iteration, which corresponds to an ASCII 'S'.  The next word corresponds to a 'V', then 'U', then 'S', then 'C', then 'G', and so on.

## Script

The patched 'chal' binary or any custom script that implements Busy Beaver, tuned to challenge #7410754 will be able to print the states.  Here is a modified version of the [python script](solve.py) from the explanation site. It runs the Turing Machine until there are more ones than any ASCII character.

    ./solve.py 5

Will show you all the needed states with indices so you can just count the ones on the required lines.

## Flag
SVUSCG{d@m\_g0OD\_wOrK\_Wo0d\_u\_aGr33?\_7410754}
