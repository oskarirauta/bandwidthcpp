#include <iostream>
#include <chrono>
#include <thread>
#include "constants.hpp"
#include "cmdparser.hpp"
#include "bandwidth.hpp"

std::string ifd_name;
bool list_ifds = false;
bool kbps = false;
bool keep_unit = false;
int interval = 1000;

static void version_header(void) {

	std::cout << APP_NAME << " version " << APP_VERSION << "\n"
		"author: Oskari Rauta" << std::endl;
}

static void usage(const CmdParser::Arg &arg) {

	std::cout << "\nusage: " << arg.cmd << " [args]" << "\n" << std::endl;
	std::cout << "options:\n" <<
		" -h, --h               usage\n" <<
		" -v, --v               version\n" <<
		" -l, --l               list available interfaces\n" <<
		" -b, --b               display kbps instead of KBps\n" <<
		" -k, --k               keep units, do not convert kb to mb even when number grows\n" <<
		" -d, --d <ms>          interval, 500-4000 as milliseconds, closest to 1s (1000) is most accurate\n" <<
		" -i, --i <interface>   selects interface to monitor\n" <<
		std::endl;
}

static void show_version(const CmdParser::Arg &arg) {

	version_header();
	exit(0);
}

static void select_ifd(const CmdParser::Arg &argv) {
	ifd_name = argv.var;
}

static void select_delay(const CmdParser::Arg &argv) {

	if ( argv.arg.empty()) {
		std::cout << "illegal interval selected. Value must be between 500 and 4000. Value was empty." << std::endl;
		exit(1);
	}

	int d = 0;

	for ( char ch : argv.var ) {

		if ( !isdigit(ch)) {
			std::cout << "illegal interval selected. Only digits are allowed as a value." << std::endl;
			exit(1);
		}

		d = ( d * 10 ) + ch - 48;
		if ( d > 4000 ) {
			std::cout << "illegal interval selected. Values must not exceed 4000." << std::endl;
			exit(1);
		}
	}

	if ( d < 500 || d > 4000 ) {
		std::cout << "illegal interval selected. Value must be between 500 and 4000." << std::endl;
		exit(1);
	}

	interval = d;
}

int main(int argc, char **argv) {

	CmdParser cmdparser(argc, argv,
		{
			{{ "-h", "--h", "-help", "--help" }, [](const CmdParser::Arg &arg) {
				version_header();
				usage(arg);
				exit(0);
			}, false },
			{{ "-v", "--v", "-version", "--version" }, show_version },
			{{ "-i", "--i", "-interface", "--interface" }, select_ifd, true },
			{{ "-l", "--l", "-list", "--list" }, [](const CmdParser::Arg &arg) {
				list_ifds = true;
			}},
			{{ "-b", "--b", "-B", "--B" }, [](const CmdParser::Arg &arg) {
				kbps = true;
			}},
			{{ "-k", "--k", "-keep", "--keep" }, [](const CmdParser::Arg &arg) {
				keep_unit = true;
			}},
			{{ "-d", "--d", "-interval", "--interval", "-delay", "--delay" }, select_delay, true },
			{{ "" }, [](const CmdParser::Arg &arg) {
				std::cout << "unknown argument " << arg.arg << "\n" <<
					"Try executing " << arg.cmd << " --h for usage" <<
					std::endl;
			}}
		});

	cmdparser.parse();

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

	bool ok = bm.update();
	if ( !ok ) {
		std::cout << "error: failed to open/read file /proc/net/dev\nAborted" << std::endl;
		return 1;
	}

	uint64_t rx = 0;
	uint64_t tx = 0;
	bool first = true;

	while ( true ) {

		std::this_thread::sleep_for (std::chrono::milliseconds(interval));

		if (!bm.update()) {
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
			uint64_t new_tx = ifd.rx_rate() / 1024;

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

			// not perfect, comparison with 1024kb and 1024mb returns true, but accurate enough for this example
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
