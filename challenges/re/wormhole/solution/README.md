# Wormhole

## Solution

The layout of the binary makes it difficult to track the validation of the flag. Each character of the flag is passed into a rune channel, which is then read by a validator function which validates a single character and passes any failing results into a bool channel called "wormhole". At the end of the execution, the wormhole channel selects a result to see if any false values are in the channel. If so, it knows the validation failed.

Players have a few different paths to follow, either by statically analyzing the binary to find the ins and outs of each of the validator channels and their associated logic, or by attempting to trace the execution to see where the flag is validated. If they are able to trace the execution either way, then they should be able to see where each character they entered undergoes mathametical functions and then is compared with a stored check value. By putting in known values and tracing how they are transformed and compared, they can recover what the flag is supposed to be.