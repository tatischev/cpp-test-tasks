# Object pool samples in C and C++

I implemented two tests: one is the simple pool in pure C. It implemented for Windows OS and contains CriticalSection for safety operations. 
Second test is the C++ implementation of thread safe pool using boost queue. Both implementation include tests.

## Object pool in C++

Standard pool implementation. I used one of my old implementations as a basis. For reference, I have added two functions - "acquire" and "try_acquire". The difference in functions is that they define different behaviors. The first function uses the local variable TObject<T> which causes the object to be released after exiting the function. The second function - "try_acquire" - operates with a unique pointer which allows you to manage the pool and is more safely. For testing I used GTest. First test is the multi threading implementation, second test shows the using of the pool in "manual" mode.

## Simple C pool

It is very simple implementation of pool I implemented it very fast. It uses two structures for storing objects. The functions add and get are protected by critical sections for security in a multithreaded application. 

## Contributing

the code can be copied, changed and supplemented without restrictions

## License
[MIT](https://choosealicense.com/licenses/mit/)
