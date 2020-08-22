#include "soul.h"
#include "doctest/doctest.h"


TEST_SUITE("Soul")
{
	TEST_CASE("Initialization")
	{
		const synodic::SoulParameters parameters;
		synodic::Soul soul(parameters);
	}

}