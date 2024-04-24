# capnp-image
Simple implementation of capnproto for sending image client &lt;--> server

## Compilation
```
capnp compile -oc++ echo.capnp
g++ echo.capnp.c++ client.cpp -o client -std=c++20 `pkg-config --cflags --libs capnp kj kj-async capnp-rpc opencv4`
g++ echo.capnp.c++ server.cpp -o server -std=c++20 `pkg-config --cflags --libs capnp kj kj-async capnp-rpc opencv4`
```

## Usage
`./server`
`./client`
