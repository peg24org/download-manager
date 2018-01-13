# Multi threaded download manager

Multi threaded download manager written in C++

### Compiling

```
mkdir build
```
```
cd build
```
```
cmake ..
```
```
make
```

Usage:
```
./main http://www.example.com/file.dat
```
Download with four threads:
```
./main -n 4 http://www.example.com/file.dat

```
