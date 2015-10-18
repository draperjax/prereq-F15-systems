README for CS 61 Problem Set 3
------------------------------
YOU MUST FILL OUT THIS FILE BEFORE SUBMITTING!

OTHER COLLABORATORS AND CITATIONS (if any):
CSAPP - Chapter 10, File Input/Output
CS61 - Lecture Exercise "Building a Simple Cache"

KNOWN BUGS (if any):


NOTES FOR THE GRADER (if any):

Stylistic Decisions: 
	- MMAP MAX: I chose to setup a max for the MMAP use, which after testing I settled on a 3 MB. I'm not confident this is the ideal value, but in combination with my buffer size, this saw the best times.
	- Buffer Size: I tested with many values here, 4096 performed the best so I settled with that value! Larger buffers didn't actually result in better times across the tests and I was optimizing for better performance across all the tests.  

Most Recent output on grading server (as a reference for test times):
TEST:      1. regular small file, character I/O, sequential
COMMAND:   ./cat61 files/text1meg.txt > files/out.txt
STDIO:     0.13966s (0.04800s user, 0.00000s system, 548KiB memory, 5 trials)
YOUR CODE: 0.03484s (0.01200s user, 0.00400s system, 556KiB memory, 5 trials)
RATIO:     4.01x stdio

TEST:      2. regular small binary file, character I/O, sequential
COMMAND:   ./cat61 files/binary1meg.bin > files/out.bin
STDIO:     0.07996s (0.04800s user, 0.00000s system, 548KiB memory, 5 trials)
YOUR CODE: 0.03791s (0.01600s user, 0.00000s system, 556KiB memory, 5 trials)
RATIO:     2.11x stdio

TEST:      3. regular large file, character I/O, sequential
COMMAND:   ./cat61 files/text20meg.txt > files/out.txt
STDIO:     1.63509s (0.84000s user, 0.08800s system, 548KiB memory, 5 trials)
YOUR CODE: 0.71694s (0.28400s user, 0.06800s system, 20712KiB memory, 5 trials)
RATIO:     2.28x stdio

TEST:      4. regular medium file, 1KB block I/O, sequential
COMMAND:   ./blockcat61 -b 1024 files/text5meg.txt > files/out.txt
STDIO:     0.07327s (0.00000s user, 0.02400s system, 544KiB memory, 5 trials)
YOUR CODE: 0.08707s (0.00400s user, 0.01600s system, 5332KiB memory, 5 trials)
RATIO:     0.84x stdio

TEST:      5. regular large file, 4KB block I/O, sequential
COMMAND:   ./blockcat61 files/text20meg.txt > files/out.txt
STDIO:     0.33721s (0.00800s user, 0.07600s system, 548KiB memory, 5 trials)
YOUR CODE: 0.20080s (0.01200s user, 0.07600s system, 20740KiB memory, 5 trials)
RATIO:     1.68x stdio

TEST:      6. regular small binary file, 4KB block I/O, sequential
COMMAND:   ./blockcat61 files/binary1meg.bin > files/out.bin
STDIO:     0.02831s (0.00000s user, 0.00400s system, 544KiB memory, 5 trials)
YOUR CODE: 0.02368s (0.00000s user, 0.00400s system, 552KiB memory, 5 trials)
RATIO:     1.20x stdio

TEST:      7. regular large file, 1B-4KB block I/O, sequential
COMMAND:   ./randblockcat61 files/text20meg.txt > files/out.txt
STDIO:     0.34348s (0.00800s user, 0.08400s system, 548KiB memory, 5 trials)
YOUR CODE: 0.30998s (0.02400s user, 0.06800s system, 20692KiB memory, 5 trials)
RATIO:     1.11x stdio

TEST:      8. regular large file, 1B-4KB block I/O, sequential
COMMAND:   ./randblockcat61 -r 6582 files/text20meg.txt > files/out.txt
STDIO:     0.33944s (0.02000s user, 0.07200s system, 548KiB memory, 5 trials)
YOUR CODE: 0.32129s (0.00800s user, 0.08800s system, 20692KiB memory, 5 trials)
RATIO:     1.06x stdio

TEST:      9. gathered small files, 512B block I/O, sequential
COMMAND:   ./gather61 -b 512 files/binary1meg.bin files/text1meg.txt > files/out.bin
STDIO:     0.03411s (0.00000s user, 0.00800s system, 548KiB memory, 5 trials)
YOUR CODE: 0.02852s (0.00000s user, 0.00800s system, 556KiB memory, 5 trials)
RATIO:     1.20x stdio

TEST:      10. scattered small file, 512B block I/O, sequential
COMMAND:   ./scatter61 -b 512 files/out1.txt files/out2.txt files/out3.txt < files/text1meg.txt
STDIO:     0.04205s (0.00000s user, 0.00400s system, 516KiB memory, 5 trials)
YOUR CODE: 0.01500s (0.00000s user, 0.00400s system, 524KiB memory, 5 trials)
RATIO:     2.80x stdio

TEST:      11. regular medium file, character I/O, reverse order
COMMAND:   ./reverse61 files/text5meg.txt > files/out.txt
STDIO:     12.32612s (2.90800s user, 5.42800s system, 548KiB memory, 2 trials)
YOUR CODE: 0.21298s (0.10800s user, 0.01600s system, 5340KiB memory, 5 trials)
RATIO:     57.87x stdio

TEST:      12. regular large file, character I/O, reverse order
COMMAND:   ./reverse61 files/text20meg.txt > files/out.txt
STDIO:     40.38563s (11.91200s user, 20.57200s system, 544KiB memory, 1 trial)
YOUR CODE: 0.93360s (0.42800s user, 0.07200s system, 20720KiB memory, 4 trials)
RATIO:     43.26x stdio

TEST:      13. seekable unmappable file, character I/O, sequential
COMMAND:   ./cat61 -s 4096 /dev/urandom > files/out.txt
STDIO:     0.00798s (0.00000s user, 0.00000s system, 544KiB memory, 5 trials)
YOUR CODE: 0.01028s (0.00000s user, 0.00000s system, 552KiB memory, 5 trials)
RATIO:     0.78x stdio

TEST:      14. seekable unmappable file, character I/O, reverse order
COMMAND:   ./reverse61 -s 4096 /dev/urandom > files/out.txt
STDIO:     0.01551s (0.00000s user, 0.00800s system, 548KiB memory, 5 trials)
YOUR CODE: 0.01483s (0.00400s user, 0.00000s system, 552KiB memory, 5 trials)
RATIO:     1.05x stdio

TEST:      15. magic zero file, character I/O, sequential
COMMAND:   ./cat61 -s 5242880 /dev/zero > files/out.txt
STDIO:     0.24427s (0.18000s user, 0.04400s system, 548KiB memory, 5 trials)
YOUR CODE: 0.09933s (0.06400s user, 0.01200s system, 556KiB memory, 5 trials)
RATIO:     2.46x stdio

TEST:      16. magic zero file, character I/O, reverse order
COMMAND:   ./reverse61 -s 5242880 /dev/zero > files/out.txt
STDIO:     21.05625s (5.43200s user, 12.56400s system, 548KiB memory, 2 trials)
YOUR CODE: 9.30968s (2.46800s user, 4.69600s system, 552KiB memory, 1 trial)
RATIO:     2.26x stdio

TEST:      17. regular large file, 4KB block I/O, random seek order
COMMAND:   ./reordercat61 files/text20meg.txt > files/out.txt
STDIO:     2.01608s (0.00000s user, 0.21200s system, 544KiB memory, 5 trials)
YOUR CODE: 0.29873s (0.02400s user, 0.08400s system, 20752KiB memory, 5 trials)
RATIO:     6.75x stdio

TEST:      18. regular large file, 4KB block I/O, random seek order
COMMAND:   ./reordercat61 -r 6582 files/text20meg.txt > files/out.txt
STDIO:     1.97689s (0.03200s user, 0.16400s system, 544KiB memory, 5 trials)
YOUR CODE: 0.33561s (0.01200s user, 0.09600s system, 20764KiB memory, 5 trials)
RATIO:     5.89x stdio

TEST:      19. regular medium file, character I/O, 1MB stride order
COMMAND:   ./stridecat61 -t 1048576 files/text5meg.txt > files/out.txt
STDIO:     24.72228s (5.94400s user, 15.27600s system, 548KiB memory, 1 trial)
YOUR CODE: 0.46644s (0.27200s user, 0.00800s system, 5316KiB memory, 5 trials)
RATIO:     53.00x stdio

TEST:      20. regular medium file, character I/O, 2B stride order
COMMAND:   ./stridecat61 -t 2 files/text5meg.txt > files/out.txt
STDIO:     9.47311s (3.53600s user, 5.26800s system, 548KiB memory, 3 trials)
YOUR CODE: 0.42469s (0.23600s user, 0.03200s system, 5316KiB memory, 5 trials)
RATIO:     22.31x stdio

TEST:      21. piped small file, character I/O, sequential
COMMAND:   cat files/text1meg.txt | ./cat61 | cat > files/out.txt
STDIO:     0.07264s (0.04400s user, 0.00000s system, 544KiB memory, 5 trials)
YOUR CODE: 0.04904s (0.00800s user, 0.00800s system, 544KiB memory, 5 trials)
RATIO:     1.48x stdio

TEST:      22. piped large file, character I/O, sequential
COMMAND:   cat files/text20meg.txt | ./cat61 | cat > files/out.txt
STDIO:     1.29865s (0.82000s user, 0.10000s system, 544KiB memory, 5 trials)
YOUR CODE: 0.84066s (0.21600s user, 0.10800s system, 544KiB memory, 4 trials)
RATIO:     1.54x stdio

TEST:      23. piped medium file, 1KB block I/O, sequential
COMMAND:   cat files/text5meg.txt | ./blockcat61 -b 1024 | cat > files/out.txt
STDIO:     0.06788s (0.00400s user, 0.01200s system, 544KiB memory, 5 trials)
YOUR CODE: 0.06907s (0.00000s user, 0.01200s system, 548KiB memory, 5 trials)
RATIO:     0.98x stdio

TEST:      24. piped large file, 4KB block I/O, sequential
COMMAND:   cat files/text20meg.txt | ./blockcat61 | cat > files/out.txt
STDIO:     0.31827s (0.02400s user, 0.04400s system, 544KiB memory, 5 trials)
YOUR CODE: 0.30603s (0.01200s user, 0.04800s system, 544KiB memory, 5 trials)
RATIO:     1.04x stdio

TEST:      25. piped large file, 1B-4KB block I/O, sequential
COMMAND:   cat files/text20meg.txt | ./randblockcat61 | cat > files/out.txt
STDIO:     0.33294s (0.02000s user, 0.05600s system, 544KiB memory, 5 trials)
YOUR CODE: 0.30670s (0.00800s user, 0.06400s system, 548KiB memory, 5 trials)
RATIO:     1.09x stdio

SUMMARY:   25 tests, 0 killed, 0 errors
           better than stdio 22 times, worse 3 times
           average 8.80x stdio
           total time 117.367s stdio, 15.454s your code (7.59x stdio)