# LKmalloc

A malloc-debugging library.
A wrapper around malloc.

## Compile and Running Testsuite

### Compiling

Inside of the 'TestSuite' directory, type:

`$ make clean all`

Which will compile the main driver program.

### Running

To run the testsuite, after compiling, type:

`$ ./testsuite`

Which will run several test programs.

### Tests

To run the regression tests (smaller independent tests)

`$ make clean test`

Which will run all of the test programs, otherwise - you can run each test file independently by locating the files in the `/tests/` directory.

## Directories

### LKmalloc/

The directory LKmalloc contains the source code for my implementations of LKmalloc, LKfree, and LKreport.

### TestSuite/

The directory TestSuite contains many small sample test programs that compile and use the LKmalloc library.

### TestSuite/tests/

Has all of the test `.sh` scripts which execute the compiled test programs.

## Approach

### Design

Since it was necessary to keep track of all the malloc's and free's done - I needed a way of storing that information. Additionally, since it appears that I would not be deleting records, and every record created could potentially be used by a call to `lkreport`, I decided that it wasn't necessary to go with a complex data structure that'd need be sorted or items deleted. I ended up creating an adapatation of a linked list for my data structure. One linked list for keeping track of free's (`FREES_TABLE`) and one for keeping track of malloc's (`MALLOCS_TABLE`).

### LKmalloc

For the malloc wrapper - it initially creates a record with the linked list; and then malloc's the requested amount of space. It updates the record with the new ptr address. If LKM_INIT flag is given, the function will recursively call itself but without the init flag (`flags & ~LMK_INIT`) - then in the original function it'll memset once the regular malloc is done.

### LKfree

LKfree has two main blocks - one of which is meant of LKF_APPROX, where the pointer address given isn't necessarily the exact address of the malloc'd space. What happens here - is that all the malloc record addresses are iterated through to check if their address & size overlap with the pointer address given. Simply, if the pointer given the LKfree was `0x0101`, and a malloc pointer has the address of `0x0100` and a size of 8, then the malloc address range goes up to `0x0108` (non inclusive); so simply check whether the sum of the size of the malloc and its pointer address overlap with the provided free pointer.

### LKreport

With LKreport - since there are multiple different types of reports that can be generated - and multiple flags can be passed at a time; the function recursively calls itself, whilst removing and printing the ouput of 1 flag at a time until there are no more flags left - aka; no more reports to be made.
