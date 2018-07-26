#define CATCH_CONFIG_RUNNER
#include <Catch2/catch.hpp>

#include <conio.h>


int main(int argc, char* argv[]) {
	int result = Catch::Session().run(argc, argv);

	//_getch();

	return result;
}
