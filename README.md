# Multi connection download manager

Multi threaded download manager written in C++

### Build

```
git submodule init
```
```
git submodule update
```
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
