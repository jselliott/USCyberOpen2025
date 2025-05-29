# FGASLR

## Solution

This challenge is a unique implementation of an FGASLR technique to map each function at a random page. The challenge itself allows for a simple tic tac toe game, the vulnerability is that there is no bounds check when writing the character. At the moment the character the user can write is only an X or O, but with careful targeting this can be modified.

First is to examine the memory layout of the challenge, even though functions are at different pages, we have a identifiable memory layout. First page is a list of function pointers, this relates to a Global Offset Table (GOT) but for individual functions. The second page is the text section of the function, the last page is the bss. 

Looking through the implementation there appears to be no way to get any leaks to previous sections, limiting us to only looking forward. Looking through the lists we can find a win function as our primary target. The only problem is that this function is only loaded into a nested location we currently don't have a leak for.

Back to the memory layout, our current looping function is limited to 9 moves or until a win is determined. Let's take at where all of this is stored in memory. The bss of our current loop function does not contain anything we want, but if we look at the bss of the parent function we can see the board struct in memory. This contains the board itself, an input buffer, the move lookup board, and the move counter.

Looking at the usage of these variables we will see that the current character placed in memory is based off current move. We can use this along with our out of bounds indexing to overwrite the moves board, the counter, and the board pointer itself within the board struct. This allows us 3 things, First: move the moves board directly over the input buffer, change the move index to reference the input buffer, and to arb read/write pointers with the board pointer.

Using these primitives we can now leak our current pointers, and start to leak the next pointer to get to our main loop function, play_game. As we don't have the ability to write to our board all at once and are limited to 1 byte at a time we can't directly jump to the next page yet.  Instead we need to overwrite a base pointer at the top of the GOT page to a new offset of a fake board struct and then overwrite the exit function to recall our current page. This allows the next loop to have control of the board pointer to start and leak the next page. This can be accomplished once more to finally leak the win function and overwrite one last time and get the flag
