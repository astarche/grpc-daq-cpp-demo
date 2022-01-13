### Build

```
git clone https://github.com/astarche/grpc-daq-cpp-demo.git
git submodule update --init --recursive
mkdir build
cd build
cmake ..
cmake --build .
```

### Server setup

Follow [instructions from grpc-device](https://github.com/ni/grpc-device#downloading-a-release).

### Run Client

```
.\grpc_daq_cpp_demo.exe localhost:31763 gRPCSystemTestDAQ/ai0
```

### Sample Output

```
Read 1000 Samples.
First data point: 4.85031
```
