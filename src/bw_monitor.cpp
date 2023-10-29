#include <chrono>
#include "bandwidth.hpp"

bandwidth::monitor::monitor() {

	this -> update();
}

bandwidth::monitor::~monitor() {

	this -> fd.close();
}

std::list<bandwidth::interface> bandwidth::monitor::interfaces() {

	return this -> _interfaces;
}

static bool parse_line(const std::string &line, std::string &ifd_name, unsigned long long &rxb, unsigned long long &rxp, unsigned long long &rxe,
								unsigned long long &txb, unsigned long long &txp, unsigned long long &txe) {

	int idx = 0;
	std::string value;
	unsigned long long n = 0;

	rxb = 0;
	rxp = 0;
	txb = 0;
	txp = 0;
	rxe = 0;
	txe = 0;

	for ( char ch : line ) {

		if ( ch == '|' )
			return false;
		if ( value.empty() && ( ch == ' ' || ch == '\t' ))
			continue;

		if ( idx == 0 && ch == ':' ) {
			ifd_name = value;
			if ( ifd_name.empty())
				return false;
			value = "";
			idx++;
			continue;
		}

		if ( idx != 0 && ( ch == ' ' || ch == '\t' )) {
			// got val
			switch ( idx ) {
				case 1: rxb = n; break;
				case 2: rxp = n; break;
				case 3: rxe = n; break;
				case 9: txb = n; break;
				case 10: txp = n; break;
				case 11: txe = n; break;
				default:;
			}
			if ( idx == 11 )
				return ifd_name.empty() ? false : true;
			n = 0;
			value = "";
			idx++;
			continue;
		}

		if ( idx == 0 && ch != ' ' && ch != '\t' )
			value += ch;
		else if ( idx != 0 && !isdigit(ch))
			return false;
		else if ( idx != 0 ) {
			n = ( n * 10 ) + ch - 48;
			value += ch;
		}
	}

	return ifd_name.empty() ? false : true;
}

bool bandwidth::monitor::update(void) {

	if ( this -> fd.is_open())
		this -> fd.close();

	if ( this -> fd.open("/proc/net/dev"); (!this -> fd.is_open() || !this -> fd.good()))
		return false;

	std::string line;

	while ( !this -> fd.eof() && std::getline(this -> fd, line)) {

		std::string ifd_name;
		unsigned long long rxb, txb, rxp, txp, rxe, txe;
		if ( !parse_line(line, ifd_name, rxb, rxp, rxe, txb, txp, txe))
			continue;

		bool updated = false;

		for ( auto &ifd : this -> _interfaces ) {
			if ( ifd._name == ifd_name ) {
				ifd.update(rxb, rxp, rxe, txb, txp, txe);
				updated = true;
			}
		}

		if ( !updated )
			this -> _interfaces.push_back(bandwidth::interface(
					ifd_name,
					rxb, rxp, rxe,
					txb, txp, txe));

	}

	this -> fd.close();
	return true;
}
