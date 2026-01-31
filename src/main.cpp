#include "core.hpp"

#include <cstdlib>
#include <exception>
#include <print>

int main(void)
{
	try {
		Core app;
		app.run();
	}
	catch (const std::exception& e)
	{
		std::println("{}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
