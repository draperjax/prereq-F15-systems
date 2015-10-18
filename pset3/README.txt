README for CS 61 Problem Set 3
------------------------------
YOU MUST FILL OUT THIS FILE BEFORE SUBMITTING!

OTHER COLLABORATORS AND CITATIONS (if any):



KNOWN BUGS (if any):



NOTES FOR THE GRADER (if any):

DO NOT GRADE: APPLYING LATE HOURS


MOST RECENT OUTPUT ON MY LOCAL MACHINE:
jharvard@appliance (~/Dropbox/CS61/jishajacob-cs61-psets/pset3): make check
MAKE=1 perl check.pl
TEST:      1. regular small file, character I/O, sequential
COMMAND:   ./cat61 files/text1meg.txt > files/out.txt
STDIO:     0.05983s (0.02400s user, 0.00000s system, 412KiB memory, 5 trials)
  COMPILE io61.c
  LINK cat61
YOUR CODE: 0.01985s (0.00800s user, 0.00000s system, 280KiB memory, 5 trials)
RATIO:     3.01x stdio

TEST:      2. regular small binary file, character I/O, sequential
COMMAND:   ./cat61 files/binary1meg.bin > files/out.bin
STDIO:     0.06046s (0.02800s user, 0.00000s system, 416KiB memory, 5 trials)
YOUR CODE: 0.01906s (0.00800s user, 0.00000s system, 276KiB memory, 5 trials)
RATIO:     3.17x stdio

TEST:      3. regular large file, character I/O, sequential
COMMAND:   ./cat61 files/text20meg.txt > files/out.txt
STDIO:     1.00804s (0.46800s user, 0.02800s system, 412KiB memory, 3 trials)
YOUR CODE: 0.32420s (0.13600s user, 0.01600s system, 20868KiB memory, 5 trials)
RATIO:     3.11x stdio

TEST:      4. regular medium file, 1KB block I/O, sequential
COMMAND:   ./blockcat61 -b 1024 files/text5meg.txt > files/out.txt
STDIO:     0.01703s (0.00000s user, 0.00400s system, 432KiB memory, 5 trials)
  LINK blockcat61
YOUR CODE: 0.01564s (0.00400s user, 0.00000s system, 5288KiB memory, 5 trials)
RATIO:     1.09x stdio

TEST:      5. regular large file, 4KB block I/O, sequential
COMMAND:   ./blockcat61 files/text20meg.txt > files/out.txt
STDIO:     0.07129s (0.00800s user, 0.01600s system, 420KiB memory, 5 trials)
YOUR CODE: 0.05948s (0.00400s user, 0.01200s system, 20864KiB memory, 5 trials)
RATIO:     1.20x stdio

TEST:      6. regular small binary file, 4KB block I/O, sequential
COMMAND:   ./blockcat61 files/binary1meg.bin > files/out.bin
STDIO:     0.00311s (0.00000s user, 0.00000s system, 420KiB memory, 5 trials)
YOUR CODE: 0.00275s (0.00000s user, 0.00000s system, 276KiB memory, 5 trials)
RATIO:     1.13x stdio

TEST:      7. regular large file, 1B-4KB block I/O, sequential
COMMAND:   ./randblockcat61 files/text20meg.txt > files/out.txt
STDIO:     0.06431s (0.00400s user, 0.02000s system, 428KiB memory, 5 trials)
  LINK randblockcat61
YOUR CODE: 0.05425s (0.00800s user, 0.00800s system, 20868KiB memory, 5 trials)
RATIO:     1.19x stdio

TEST:      8. regular large file, 1B-4KB block I/O, sequential
COMMAND:   ./randblockcat61 -r 6582 files/text20meg.txt > files/out.txt
STDIO:     0.06062s (0.00400s user, 0.01600s system, 436KiB memory, 5 trials)
YOUR CODE: 0.05385s (0.00400s user, 0.01600s system, 20864KiB memory, 5 trials)
RATIO:     1.13x stdio

TEST:      9. gathered small files, 512B block I/O, sequential
COMMAND:   ./gather61 -b 512 files/binary1meg.bin files/text1meg.txt > files/out.bin
STDIO:     0.00979s (0.00000s user, 0.00000s system, 436KiB memory, 5 trials)
  LINK gather61
YOUR CODE: 0.00747s (0.00000s user, 0.00000s system, 280KiB memory, 5 trials)
RATIO:     1.31x stdio

TEST:      10. scattered small file, 512B block I/O, sequential
COMMAND:   ./scatter61 -b 512 files/out1.txt files/out2.txt files/out3.txt < files/text1meg.txt
STDIO:     0.00706s (0.00000s user, 0.00000s system, 440KiB memory, 5 trials)
  LINK scatter61
YOUR CODE: 0.00670s (0.00000s user, 0.00000s system, 276KiB memory, 5 trials)
RATIO:     1.05x stdio

TEST:      11. regular medium file, character I/O, reverse order
COMMAND:   ./reverse61 files/text5meg.txt > files/out.txt
STDIO:     1.89749s (0.39600s user, 0.54400s system, 420KiB memory, 2 trials)
  LINK reverse61
YOUR CODE: 0.12201s (0.05600s user, 0.00400s system, 5288KiB memory, 5 trials)
RATIO:     15.55x stdio

TEST:      12. regular large file, character I/O, reverse order
COMMAND:   ./reverse61 files/text20meg.txt > files/out.txt
STDIO:     7.55888s (1.68800s user, 2.05600s system, 420KiB memory, 1 trial)
YOUR CODE: 0.44773s (0.20400s user, 0.01200s system, 20864KiB memory, 5 trials)
RATIO:     16.88x stdio

TEST:      13. seekable unmappable file, character I/O, sequential
COMMAND:   ./cat61 -s 4096 /dev/urandom > files/out.txt
STDIO:     0.00391s (0.00000s user, 0.00000s system, 436KiB memory, 5 trials)
YOUR CODE: 0.00455s (0.00000s user, 0.00000s system, 280KiB memory, 5 trials)
RATIO:     0.86x stdio

TEST:      14. seekable unmappable file, character I/O, reverse order
COMMAND:   ./reverse61 -s 4096 /dev/urandom > files/out.txt
STDIO:     0.00802s (0.00000s user, 0.00000s system, 444KiB memory, 5 trials)
YOUR CODE: 0.00620s (0.00000s user, 0.00000s system, 276KiB memory, 5 trials)
RATIO:     1.29x stdio

TEST:      15. magic zero file, character I/O, sequential
COMMAND:   ./cat61 -s 5242880 /dev/zero > files/out.txt
STDIO:     0.23456s (0.11200s user, 0.00000s system, 436KiB memory, 5 trials)
YOUR CODE: 0.08766s (0.02800s user, 0.01200s system, 276KiB memory, 5 trials)
RATIO:     2.68x stdio

TEST:      16. magic zero file, character I/O, reverse order
COMMAND:   ./reverse61 -s 5242880 /dev/zero > files/out.txt
STDIO:     3.60848s (0.62400s user, 1.18000s system, 440KiB memory, 1 trial)
YOUR CODE: 0.81627s (0.17600s user, 0.22800s system, 276KiB memory, 4 trials)
RATIO:     4.42x stdio

TEST:      17. regular large file, 4KB block I/O, random seek order
COMMAND:   ./reordercat61 files/text20meg.txt > files/out.txt
STDIO:     0.14381s (0.00800s user, 0.01200s system, 448KiB memory, 5 trials)
  LINK reordercat61
YOUR CODE: 0.13236s (0.00800s user, 0.00800s system, 20868KiB memory, 5 trials)
RATIO:     1.09x stdio

TEST:      18. regular large file, 4KB block I/O, random seek order
COMMAND:   ./reordercat61 -r 6582 files/text20meg.txt > files/out.txt
STDIO:     0.14516s (0.00800s user, 0.01200s system, 460KiB memory, 5 trials)
YOUR CODE: 0.16439s (0.00000s user, 0.03200s system, 20864KiB memory, 5 trials)
RATIO:     0.88x stdio

TEST:      19. regular medium file, character I/O, 1MB stride order
COMMAND:   ./stridecat61 -t 1048576 files/text5meg.txt > files/out.txt
STDIO:     6.64662s (0.95200s user, 2.34000s system, 440KiB memory, 1 trial)
  LINK stridecat61
YOUR CODE: 0.23845s (0.11600s user, 0.00000s system, 5292KiB memory, 5 trials)
RATIO:     27.87x stdio

TEST:      20. regular medium file, character I/O, 2B stride order
COMMAND:   ./stridecat61 -t 2 files/text5meg.txt > files/out.txt
STDIO:     2.48873s (0.72000s user, 0.51600s system, 440KiB memory, 2 trials)
YOUR CODE: 0.19344s (0.09600s user, 0.00000s system, 5288KiB memory, 5 trials)
RATIO:     12.87x stdio

TEST:      21. piped small file, character I/O, sequential
COMMAND:   cat files/text1meg.txt | ./cat61 | cat > files/out.txt
STDIO:     0.05690s (0.02000s user, 0.00000s system, 412KiB memory, 5 trials)
YOUR CODE: 0.02640s (0.00800s user, 0.00000s system, 280KiB memory, 5 trials)
RATIO:     2.16x stdio

TEST:      22. piped large file, character I/O, sequential
COMMAND:   cat files/text20meg.txt | ./cat61 | cat > files/out.txt
STDIO:     1.02056s (0.40800s user, 0.05200s system, 412KiB memory, 3 trials)
YOUR CODE: 0.36040s (0.10800s user, 0.02400s system, 276KiB memory, 5 trials)
RATIO:     2.83x stdio

TEST:      23. piped medium file, 1KB block I/O, sequential
COMMAND:   cat files/text5meg.txt | ./blockcat61 -b 1024 | cat > files/out.txt
STDIO:     0.02019s (0.00000s user, 0.00000s system, 436KiB memory, 5 trials)
YOUR CODE: 0.02138s (0.00000s user, 0.00000s system, 276KiB memory, 5 trials)
RATIO:     0.94x stdio

TEST:      24. piped large file, 4KB block I/O, sequential
COMMAND:   cat files/text20meg.txt | ./blockcat61 | cat > files/out.txt
STDIO:     0.07758s (0.00000s user, 0.00800s system, 420KiB memory, 5 trials)
YOUR CODE: 0.06694s (0.00400s user, 0.00400s system, 276KiB memory, 5 trials)
RATIO:     1.16x stdio

TEST:      25. piped large file, 1B-4KB block I/O, sequential
COMMAND:   cat files/text20meg.txt | ./randblockcat61 | cat > files/out.txt
STDIO:     0.08064s (0.00000s user, 0.00800s system, 424KiB memory, 5 trials)
YOUR CODE: 0.06784s (0.00800s user, 0.00000s system, 276KiB memory, 5 trials)
RATIO:     1.19x stdio

SUMMARY:   25 tests, 0 killed, 0 errors
           better than stdio 22 times, worse 3 times
           average 4.40x stdio
           total time 25.353s stdio, 3.319s your code (7.64x stdio)