#include <iostream>
#include <chrono>
#include <thread>
#include "bandwidth.hpp"

int main(int argc, char **argv) {

	std::cout << "bandwidth monitor example" << std::endl;

	bandwidth::monitor bm;

	bool ok = bm.update();
	if ( !ok ) {
		std::cout << "error: failed to open/read file /proc/net/dev\nAborted" << std::endl;
		return 1;
	}

	uint64_t rx = 0;
	uint64_t tx = 0;
	bool first = true;

	while ( ok ) {

		std::this_thread::sleep_for (std::chrono::milliseconds(995));
		ok = bm.update();

		for ( auto &ifd : bm.interfaces()) {

			if ( ifd.name() != "br-lan" )
				continue;

			bool m_rx = false;
			bool m_tx = false;

			uint64_t new_rx = ifd.rx_rate() / 1024;
			uint64_t new_tx = ifd.rx_rate() / 1024;

			new_rx *= 8;
			new_tx *= 8;

			if ( new_rx > 1280 ) {
				m_rx = true;
				new_rx /= 1024;
			}

			if ( new_tx > 1280 ) {
				m_tx = true;
				new_tx /= 1024;
			}

			// not perfect, comparison with 1024kb and 1024mb returns true, but accurate enough for this example
			if ( rx != new_rx || tx != new_tx || first ) {
				rx = new_rx;
				tx = new_tx;
				//std::cout << ifd.name() << " receive: " << rx << ( m_rx ? "M" : "K" ) << "B/s transmit: " << tx << ( m_tx ? "M" : "K" ) << "B/s" << std::endl;
				std::cout << ifd.name() << " receive: " << rx << ( m_rx ? "M" : "K" ) << "bps transmit: " << tx << ( m_tx ? "M" : "K" ) << "bps" << std::endl;
				first = false;
			}
		}
	}

	return 0;
}
