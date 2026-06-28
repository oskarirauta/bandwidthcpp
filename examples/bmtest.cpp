#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cstdint>

#include "constants.hpp"
#include "usage.hpp"
#include "bandwidth.hpp"

static void version_header(void) {

	std::cout << APP_NAME << " version " << APP_VERSION << "\n"
		"author: Oskari Rauta" << std::endl;
}

int main(int argc, char **argv) {

	usage_t usage = {
		.args = { argc, argv },
		.info = {
			.name = APP_NAME,
			.version = APP_VERSION,
			.author = "Oskari Rauta",
			.description = "\nMonitors the bandwidth of a network interface.\n"
		},
		.options = {
			{ "help",      { .key = "h", .word = "help", .desc = "show usage" }},
			{ "version",   { .key = "v", .word = "version", .desc = "show version" }},
			{ "list",      { .key = "l", .word = "list", .desc = "list available interfaces" }},
			{ "interface", { .key = "i", .word = "interface", .desc = "interface to monitor", .flag = usage_t::REQUIRED, .name = "interface" }},
			{ "bits",      { .key = "b", .word = "bits", .desc = "display kbps (kilobits) instead of KBps (kilobytes)" }},
			{ "keep",      { .key = "k", .word = "keep", .desc = "keep units, do not scale kb up to mb as the number grows" }},
			{ "delay",     { .key = "d", .word = "delay", .desc = "sample interval in milliseconds, 500-4000 (default 1000)", .flag = usage_t::REQUIRED, .name = "ms", .type = usage_t::INT }}
		}
	};

	if ( usage["help"] ) {
		version_header();
		std::cout << usage << "\n" << usage.help() << std::endl;
		return 0;
	}

	if ( usage["version"] ) {
		version_header();
		return 0;
	}

	if ( !usage.validated ) {

		auto errors = usage.errors();
		std::cout << "command-line errors:\n" << errors << std::endl;

		if ( std::any_of(errors.begin(), errors.end(), [](const usage_t::error_t& e) {
				return e.error != usage_t::error_type::DUPLICATE && e.error != usage_t::error_type::UNKNOWN_OPTION;
			})) {
			std::cout << "\naborting due to fatal command-line errors." << std::endl;
			return 1;
		}
	}

	bool list_ifds = usage["list"];
	bool kbps = usage["bits"];	// default is KBps (kilobytes); -b switches to kbps (kilobits)
	bool keep_unit = usage["keep"];
	int interval = 1000;
	std::string ifd_name = usage["interface"] ? (std::string)usage["interface"] : "";

	if ( usage["delay"] ) {

		long d = usage["delay"].intValue();
		if ( d < 500 || d > 4000 ) {
			std::cout << "illegal interval selected. Value must be between 500 and 4000 milliseconds." << std::endl;
			return 1;
		}
		interval = (int)d;
	}

	bool ifd_ok = false;
	bandwidth::monitor bm;

	if ( !bm.update()) {
		std::cout << "error: failed to open/read file /proc/net/dev\nAborted" << std::endl;
		return 1;
	}

	if ( list_ifds ) {
		version_header();
		std::cout << "\nlist of available interfaces:";
	}

	for ( auto &ifd : bm.interfaces()) {

		if ( !ifd_name.empty() && ifd.name() == ifd_name )
			ifd_ok = true;

		if ( list_ifds ) std::cout << " " << ifd.name();
	}

	if ( list_ifds ) {

		std::cout << std::endl;
		return 0;

	} else if ( ifd_name.empty()) {
		std::cout << "error: interface was not defined. Use -i to select interface, or -l to list available interfaces." << std::endl;
		return 1;
	} else if ( !ifd_ok ) {
		std::cout << "error: interface \"" << ifd_name << "\" was not found. Use -l argument to list available interfaces." << std::endl;
		return 1;
	}

	version_header();
	std::cout << std::endl;

	if ( !bm.update()) {
		std::cout << "error: failed to open/read file /proc/net/dev\nAborted" << std::endl;
		return 1;
	}

	uint64_t rx = 0;
	uint64_t tx = 0;
	bool first = true;

	while ( true ) {

		std::this_thread::sleep_for(std::chrono::milliseconds(interval));

		if ( !bm.update()) {
			std::cout << "error: failed to open/read file /proc/net/dev\nAborted" << std::endl;
			return 1;
		}

		ifd_ok = false;

		for ( auto &ifd : bm.interfaces()) {

			if ( ifd.name() != ifd_name )
				continue;

			ifd_ok = true;
			bool m_rx = false;
			bool m_tx = false;

			uint64_t new_rx = ifd.rx_rate() / 1024;
			uint64_t new_tx = ifd.tx_rate() / 1024;

			if ( kbps ) {
				new_rx *= 8;
				new_tx *= 8;
			}

			if ( !keep_unit ) {
				if ( new_rx > 1280 ) {
					m_rx = true;
					new_rx /= 1024;
				}

				if ( new_tx > 1280 ) {
					m_tx = true;
					new_tx /= 1024;
				}
			}

			// not perfect: comparison with 1024kb and 1024mb returns true, but
			// accurate enough for this example
			if ( rx != new_rx || tx != new_tx || first ) {

				rx = new_rx;
				tx = new_tx;

				std::cout << ifd.name() <<
					" rx: " << rx << ( m_rx ? "M" : "K" ) << ( kbps ? "bps" : "b/s" ) <<
					" tx: " << tx << ( m_tx ? "M" : "K" ) << ( kbps ? "bps" : "b/s" ) <<
					std::endl;
				first = false;
			}
		}

		if ( !ifd_ok ) {
			std::cout << "error: interface \"" << ifd_name << "\" is not available.\nAborted." << std::endl;
			return -1;
		}
	}

	return 0;
}
