#include "BakedGIApp.hh"

int main(int argc, char* argv[]) {
	BakedGIApp sample;
	return sample.run(argc, argv); // automatically sets up GLOW and GLFW and everything
}
