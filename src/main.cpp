#include "app/Application.hpp"
#include <iostream>

int main() {
	Application app;
	if (!app.init()) {
		std::cerr << "Failed to initialize application\n";
		return 1;
	}
	app.run();
	return 0;
}
