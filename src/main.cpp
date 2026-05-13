#include "app/app.hpp"

#include <cstdlib>
#include <exception>
#include <print>

int main(void)
{
	try {
		App app;
		app.run();
	}
	catch (const std::exception &e)
	{
		std::println("{}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
