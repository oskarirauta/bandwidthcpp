[![License:MIT](https://img.shields.io/badge/License-MIT-blue?style=plastic)](LICENSE)
[![C++ CI build](../../actions/workflows/build.yml/badge.svg)](../../actions/workflows/build.yml)

# bandwidthcpp

minimal bandwidth monitoring for C++

Reads `/proc/net/dev` and computes per-interface rx/tx rates. A small example,
`bmtest`, monitors a chosen interface and prints its throughput.

## <sub>library</sub>

```cpp
bandwidth::monitor bm;
bm.update();                        // refresh counters, returns false on failure

for ( auto& ifd : bm.interfaces()) {
    ifd.name();
    ifd.rx_rate();  ifd.tx_rate();  // bytes/s since the previous update()
    ifd.rx_bytes(); ifd.tx_bytes();
    // ... rx_packets / rx_errors / tx_packets / tx_errors
}
```

Call `update()` periodically (about once a second is most accurate); each call
computes the rate from the delta since the previous one.

## <sub>example</sub>

See [examples/bmtest.cpp](examples/bmtest.cpp). Build it with make:

```
git clone --recursive https://github.com/oskarirauta/bandwidthcpp.git
cd bandwidthcpp
make
./bmtest -l           # list interfaces
./bmtest -i eth0      # monitor eth0
```

bmtest parses its command line with [usage_cpp](https://github.com/oskarirauta/usage_cpp).

## <sub>importing</sub>

Add bandwidthcpp as a submodule and include its `Makefile.inc`:

```
BANDWIDTHCPP_DIR := bandwidth
include bandwidth/Makefile.inc
```

Link your target with `BANDWIDTH_OBJS`.
