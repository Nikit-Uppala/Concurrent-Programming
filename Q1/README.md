To compile the file-
gcc Q1.c -lpthread

From the results of runtime of different versions of merge sort, it is clear that the normal merge sort (without threads and processes) is faster. The possible reason is a function call is more easier than process and thread creation.
Threaded merge sort is faster than merge sort with processes as creation of thread is faster than process creation.

For some values of N, the results are
N <= 5
In this case we can't compare as we directly sort the array using selection sort.

N = 10
Normal merge sort is nearly 177 times faster than merge sort with processes and 75 times faster than threaded mergeSort.

N = 15
Normal mergeSort is nearly 267 times faster than merge sort with processes(concurrent) and 150 times faster than threaded mergeSort.

If the size of array increases, normal mergeSort is much more faster than concurrent mergeSort and threaded merge sort as size increases, the recursive calls increase and the number of processes and threads to be created also increases and hence more time is required in concurrent and threaded mergeSort.