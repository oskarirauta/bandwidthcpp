#include <chrono>
#include "bandwidth.hpp"

void bandwidth::interface::update(uint64_t rxb, uint64_t rxp, uint64_t rxe,
				uint64_t txb, uint64_t txp, uint64_t txe) {

	uint64_t rxc = rxb - this -> _rx_bytes;
	uint64_t txc = txb - this -> _tx_bytes;

	this -> _rx_bytes = rxb;
	this -> _rx_packets = rxp;
	this -> _rx_errors = rxe;
	this -> _tx_bytes = txb;
	this -> _tx_packets = txp;
	this -> _tx_errors = txe;

	if ( this -> millis == 0 ) {
		this -> _rx_rate = 0;
		this -> _tx_rate = 0;
		this -> millis = std::chrono::duration_cast<std::chrono::milliseconds>
					(std::chrono::system_clock::now().time_since_epoch()).count();
		return;
	}

	uint64_t new_millis = std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();

	uint64_t cnt = new_millis - this -> millis;

	if ( cnt == 0 )
		return;

	this -> millis = new_millis;

	if ( cnt != 1000 ) {

		double n_rxc = rxc;
		double n_txc = txc;

		// scale down
		while ( cnt >= 1000 ) {
			n_rxc *= 0.5;
			n_txc *= 0.5;
			cnt *= 0.5;
		}

		double multiplier = 1000 / cnt;

		if ( multiplier > 0 ) {

			n_rxc *= multiplier;
			n_txc *= multiplier;
			rxc = n_rxc;
			txc = n_txc;

		} else return;
	}

	this -> _rx_rate = rxc;
	this -> _tx_rate = txc;
}
