# L33tcoder

## Solution

For this challenge, the players are given a field to enter python code for their l33tcode challenge. The first thought will likely be that it is some sort of pyjail-type challenge where they need to examine the source code and find a way around restrictions in the code validation.

The app uses a pip library "uscg-leetcode-validator" to get the abstract syntax tree of the given code and then ensure that it comforms to safety standards, making it nearly impossible to bypass.

However, if they look closely they will see that the app has an injection point where they are able to change environmental variables as well as a point where the python library is updated on each submission from pip. By injecting a PIP_INDEX_URL, they are able to actually change the index where pip looks for packages to something that they control.

So, by taking the provided source code for the package and modifying it to do something like return just the contents of /flag.txt, the player can build the package and host a simulated python package repository themselves. Then inject that URL into the PIP_INDEX_URL environmental variable.

When the app collects the malicious package, it will update the local copy and then execute the code to exfiltrate the flag.