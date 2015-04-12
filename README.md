# Cuckoo hashing in C
#### Introduction


Cuckoo hashing was proposed by Pagh and Rodler (2001). The idea is to build a dictionary data structure with two hash tables and two different hash functions. The performance of applying Cuckoo hashing to a real-world problem is very
promising in terms of memory consumption and lookup time (Jaruskulchai and Kruengkrai, 2002).

ckhash is an implementation of Cuckoo hashing that can get the input in the form of strings. ckhash is written in C. The original source code can be found from this url: http://www.it-c.dk/people/pagh/papers/cuckoo.tar. You can use functions in ckhash by linking the library to your program. See `test/test-cuckoo.c' for examples of function calls.


#### Quick Start

In the ckhash base directory, type:
```
$ ./configure
$ make
```

To test that the program works properly, type:
```
$ cd test
$ ./test-cuckoo linux.words
```

You should get the following results:
```
test insert...
 45402 entries, table size = 65536
test lookup...
 0 errors
test get...
 found [ALGOL] value=0
 found [ANSI] value=1
 could not find [ansi]
test delete...
 0 errors, remain 0 entries, table size = 2
```

#### References

* Cuckoo Hashing. 
  Rasmus Pagh and Flemming Friche Rodler.
  Proceedings of ESA 2001, Lecture Notes in Computer Science, vol. 2161.

* Building Inverted Files Through Efficient Dynamic Hashing.
  Chuleerat Jaruskulchai and Canasai Kruengkrai. 
  Proceedings of the Fifth National Computer Science and Engineering Conference (NCSEC-2002).
