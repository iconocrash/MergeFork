# MergeFork

This program creates a multi-process MergeSort algorithm using fork() and Unix pipes.

The basic idea is to model the recursive process that the standard MergeSort uses, but instead of using recursion with the stack and waiting on return values we use two forks to sort the left and right subdivisions, create pipes between these two child processes and the parent, and wait to read the pipes, which will send the sorted information back up to the parent process which will, in turn, merge the results and send back to its parent (if it has a parent).

This has the advantage (over recursion) that the left and right subdivisions to be sorted will be handled in parallel for each list (including sublists) to be sorted. Thus, on a multi-processor machine the multi-process MergeSort could potentially run faster than a normal recursive MergeSort, but the overhead of using forks and pipes may counteract this (especially for the small input size of 8).

Since the fork gives each process its own independent data space, to implement our multi-process MergeSort, we simply need to copy data into the appropriate variables used by our MergeSort code for the left and right fork and be able to return to the start of our MergeSort code (done using a while loop code block and the continue statement), in effect continuing the procedure over again for the child.

The arrays for holding our left and right subdivisions need not be larger than 4 since our max input size is 8. If we were to make a more general version of this algorithm, without a capped input size we would need to use dynamic memory allocation for our subdivisions.

One limitation of note is that the numbers from the input file cannot be more than 9 digits, since in the function that parses the input datfile the numBuf array, which we use atoi() on, is hard-coded to be a size of 10 (9 digits + NULL). This limit could be raised without much difficulty however by simply making the array larger. Another limitation of note is that if this were production code, the input size of each integer from the input.dat should also be checked to make sure it would fit in the string buffer for the integer parsed from the dat file (numBuf). Otherwise, we could have a buffer overflow.
