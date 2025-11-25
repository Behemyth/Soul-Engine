#include "../../Source Files/Transput/Settings.h"
#include <cassert>
#include <iostream>
#include <cstdio>

//Tests

//Tests to make sure settings works properly when given proper inputs

namespace Settings {

	namespace detail {

		void testGetProperInputs() {
			assert(Settings::Set("a", std::int8_t(0)));
			assert(Settings::Set("b", std::uint8_t(1)));
			assert(Settings::Set("c", std::int16_t(2)));
			assert(Settings::Set("d", std::uint16_t(3)));
			assert(Settings::Set("e", std::int32_t(4)));
			assert(Settings::Set("f", std::uint32_t(5)));
			assert(Settings::Set("g", float(6.6)));
			assert(Settings::Set("h", std::int64_t(7)));
			assert(Settings::Set("i", std::uint64_t(8)));
			assert(Settings::Set("j", 9.9));

			assert(Settings::Get("a", std::int8_t(1)) == 0);
			assert(Settings::Get("b", std::uint8_t(2)) == 1);
			assert(Settings::Get("c", std::int16_t(3)) == 2);
			assert(Settings::Get("d", std::uint16_t(4)) == 3);
			assert(Settings::Get("e", std::int32_t(5)) == 4);
			assert(Settings::Get("f", std::uint32_t(6)) == 5);
			assert(Settings::Get("g", float(7.7)) == 6.6f);
			assert(Settings::Get("h", std::int64_t(8)) == 7);
			assert(Settings::Get("i", std::uint64_t(9)) == 8);
			assert(Settings::Get("j", 10.1) == 9.9);
		}

		void testGetProperInputs2() {
			assert(Settings::Set("a", std::int8_t(0)));
			assert(Settings::Set("b", std::uint8_t(1)));
			assert(Settings::Set("c", std::int16_t(2)));
			assert(Settings::Set("d", std::uint16_t(3)));
			assert(Settings::Set("e", std::int32_t(4)));
			assert(Settings::Set("f", std::uint32_t(5)));
			assert(Settings::Set("g", float(6.6)));
			assert(Settings::Set("h", std::int64_t(7)));
			assert(Settings::Set("i", std::uint64_t(8)));
			assert(Settings::Set("j", 9.9));

			std::int8_t  a; std::uint8_t  b;
			std::int16_t c; std::uint16_t d;
			std::int32_t e; std::uint32_t f; float  g;
			std::int64_t h; std::uint64_t i; double j;

			assert(Settings::Get("a", std::int8_t(1), &a));
			assert(Settings::Get("b", std::uint8_t(2), &b));
			assert(Settings::Get("c", std::int16_t(3), &c));
			assert(Settings::Get("d", std::uint16_t(4), &d));
			assert(Settings::Get("e", std::int32_t(5), &e));
			assert(Settings::Get("f", std::uint32_t(6), &f));
			assert(Settings::Get("g", float(7.7), &g));
			assert(Settings::Get("h", std::int64_t(8), &h));
			assert(Settings::Get("i", std::uint64_t(9), &i));
			assert(Settings::Get("j", 10.1, &j));

			assert(a == std::int8_t(0));
			assert(b == std::uint8_t(1));
			assert(c == std::int16_t(2));
			assert(d == std::uint16_t(3));
			assert(e == std::int32_t(4));
			assert(f == std::uint32_t(5));
			assert(g == 6.6f);
			assert(h == std::int64_t(7));
			assert(i == std::uint64_t(8));
			assert(j == 9.9);
		}

		//Tests to make sure defaultValue used when an undefined 
		//setting is retrived

		void testGetUndefinedSettings() {
			assert(Settings::Set("a", std::int8_t(0)));
			assert(Settings::Set("b", std::uint8_t(1)));
			assert(Settings::Set("c", std::int16_t(2)));
			assert(Settings::Set("d", std::uint16_t(3)));
			assert(Settings::Set("e", std::int32_t(4)));
			assert(Settings::Set("f", std::uint32_t(5)));
			assert(Settings::Set("g", float(6.6)));
			assert(Settings::Set("h", std::int64_t(7)));
			assert(Settings::Set("i", std::uint64_t(8)));
			assert(Settings::Set("j", 9.9));

			assert(Settings::Get("j", std::int8_t(1)) == 1);
			assert(Settings::Get("i", std::uint8_t(2)) == 2);
			assert(Settings::Get("h", std::int16_t(3)) == 3);
			assert(Settings::Get("g", std::uint16_t(4)) == 4);
			assert(Settings::Get("f", std::int32_t(5)) == 5);
			assert(Settings::Get("e", std::uint32_t(6)) == 6);
			assert(Settings::Get("d", float(7.7)) == 7.7f);
			assert(Settings::Get("c", std::int64_t(8)) == 8);
			assert(Settings::Get("b", std::uint64_t(9)) == 9);
			assert(Settings::Get("a", 10.1) == 10.1);
		}

		void testGetUndefinedSettings2() {
			assert(Settings::Set("a", std::int8_t(0)));
			assert(Settings::Set("b", std::uint8_t(1)));
			assert(Settings::Set("c", std::int16_t(2)));
			assert(Settings::Set("d", std::uint16_t(3)));
			assert(Settings::Set("e", std::int32_t(4)));
			assert(Settings::Set("f", std::uint32_t(5)));
			assert(Settings::Set("g", float(6.6)));
			assert(Settings::Set("h", std::int64_t(7)));
			assert(Settings::Set("i", std::uint64_t(8)));
			assert(Settings::Set("j", 9.9));

			std::int8_t  a; std::uint8_t  b;
			std::int16_t c; std::uint16_t d;
			std::int32_t e; std::uint32_t f; float  g;
			std::int64_t h; std::uint64_t i; double j;

			assert(!Settings::Get("j", std::int8_t(1), &a));
			assert(!Settings::Get("i", std::uint8_t(2), &b));
			assert(!Settings::Get("h", std::int16_t(3), &c));
			assert(!Settings::Get("g", std::uint16_t(4), &d));
			assert(!Settings::Get("f", std::int32_t(5), &e));
			assert(!Settings::Get("e", std::uint32_t(6), &f));
			assert(!Settings::Get("d", float(7.7), &g));
			assert(!Settings::Get("c", std::int64_t(8), &h));
			assert(!Settings::Get("b", std::uint64_t(9), &i));
			assert(!Settings::Get("a", 10.1, &j));

			assert(a == std::int8_t(1));
			assert(b == std::uint8_t(2));
			assert(c == std::int16_t(3));
			assert(d == std::uint16_t(4));
			assert(e == std::int32_t(5));
			assert(f == std::uint32_t(6));
			assert(g == 7.7f);
			assert(h == std::int64_t(8));
			assert(i == std::uint64_t(9));
			assert(j == 10.1);
		}

		//Serialization Tests

		void testSerialization(FileType type) {
			std::string fname = "b.ini";

			assert(Settings::Set("a", std::int8_t(0)));
			assert(Settings::Set("b", std::uint8_t(1)));
			assert(Settings::Set("c", std::int16_t(2)));
			assert(Settings::Set("d", std::uint16_t(3)));
			assert(Settings::Set("e", std::int32_t(4)));
			assert(Settings::Set("f", std::uint32_t(5)));
			assert(Settings::Set("g", float(6.6)));
			assert(Settings::Set("h", std::int64_t(7)));
			assert(Settings::Set("i", std::uint64_t(8)));
			assert(Settings::Set("j", 9.9));

			Settings::Write(fname, type);
			Settings::Read(fname, type);

			assert(Settings::Get("a", std::int8_t(1)) == 0);
			assert(Settings::Get("b", std::uint8_t(2)) == 1);
			assert(Settings::Get("c", std::int16_t(3)) == 2);
			assert(Settings::Get("d", std::uint16_t(4)) == 3);
			assert(Settings::Get("e", std::int32_t(5)) == 4);
			assert(Settings::Get("f", std::uint32_t(6)) == 5);
			assert(Settings::Get("g", float(7.7)) == 6.6f);
			assert(Settings::Get("h", std::int64_t(8)) == 7);
			assert(Settings::Get("i", std::uint64_t(9)) == 8);
			assert(Settings::Get("j", 10.1) == 9.9);

			std::remove(fname.c_str());
		}

		// Test of DeleteArchive

		void testDeleteArchive() {
			std::string fname = "testDeleteArchive.ini";
			Settings::detail::CheckArchive(fname, FileType::TEXT); // make sure there is an archive
			Settings::DeleteArchive();
			assert(Settings::detail::curArchive == nullptr);
			assert(Settings::detail::curType == FileType::null);
			std::remove(fname.c_str()); // clean up
		}

		//Runs all tests
		void runAllTests() {
			detail::testGetProperInputs();
			detail::testGetProperInputs2();
			detail::testGetUndefinedSettings();
			detail::testGetUndefinedSettings2();

			detail::testSerialization(BINARY);
			detail::testSerialization(XML);
			detail::testSerialization(TEXT);

			detail::testDeleteArchive();

			std::cout << "All tests have successfully completed." << std::endl;
		}

	}
}