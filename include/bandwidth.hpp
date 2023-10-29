#pragma once

#include <string>
#include <fstream>
#include <list>
#include <cstdint>

namespace bandwidth {

	class monitor;

	struct interface {

		private:
			std::string _name;
			uint64_t _rx_bytes;
			uint64_t _rx_packets;
			uint64_t _rx_errors;
			uint64_t _tx_bytes;
			uint64_t _tx_packets;
			uint64_t _tx_errors;
			uint64_t _rx_rate;
			uint64_t _tx_rate;
			uint64_t millis;

			interface(std::string name,
					uint64_t rxb = 0, uint64_t rxp = 0, uint64_t rxe = 0,
					uint64_t txb = 0, uint64_t txp = 0, uint64_t txe = 0) :
							_name(name),
							_rx_bytes(rxb), _rx_packets(rxp), _rx_errors(rxe),
							_tx_bytes(txb), _tx_packets(txp), _tx_errors(txe),
							_rx_rate(0), _tx_rate(0), millis(0) {};

			void update(uint64_t rxb, uint64_t rxp, uint64_t rxe,
					uint64_t txb, uint64_t txp, uint64_t txe);

		public:
			inline const std::string name() { return this -> _name; }
			inline uint64_t rx_bytes() { return this -> _rx_bytes; }
			inline uint64_t rx_packets() { return this -> _rx_packets; }
			inline uint64_t rx_errors() { return this -> _rx_errors; }
			inline uint64_t tx_bytes() { return this -> _tx_bytes; }
			inline uint64_t tx_packets() { return this -> _tx_packets; }
			inline uint64_t tx_errors() { return this -> _tx_errors; }
			inline uint64_t rx_rate() { return this -> _rx_rate; }
			inline uint64_t tx_rate() { return this -> _tx_rate; }

			friend class bandwidth::monitor;
	};

	class monitor {

		private:
			std::ifstream fd;
			std::list<bandwidth::interface> _interfaces;

		public:
			std::list<bandwidth::interface> interfaces();
			bool update(void); // returns false on failure

			monitor();
			~monitor();
	};

}
