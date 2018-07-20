#include "BakedGIApp.hh"
#include "PathTracer.hh"
#include "IlluminationBaker.hh"
#include "Scene.hh"
#include "LightMapWriter.hh"

#include <string>

// Format:
//   baked-gi <path-to-gltf> [path-to-lm] [path-to-pd]
//     OR
//   baked-gi <path-to-gltf> -bake <output-path> [BAKE_OPTIONS]
// Options:
//   -ao <w> <h> <spp> : enable ambient occlusion baking with the given width, height and samples per pixel
//   -irr <w> <h> <spp> : enable irradiance baking with the given width, height and samples per pixel
// Examples:
//   baked-gi "myscene.gltf" "prebaked.lm"
//   baked-gi "myscene.gltf" -ao 512 512 1000 -irr 256 256 2000 -f "prebaked.lm"
int main(int argc, char* argv[]) {
	std::string gltfPath(argv[1]);
	std::string lmPath;
	std::string pdPath;
	std::string outputPath;
	int aoWidth = 0, aoHeight = 0, aoSpp = 0;
	int irrWidth = 0, irrHeight = 0, irrSpp = 0;
	float lightStrength = 5.0f;

	if (argc >= 3) {
		std::string arg(argv[2]);
		if (arg == "-bake") {
			outputPath = std::string(argv[3]);

			for (int i = 4; i < argc; ) {
				std::string arg(argv[i]);
				if (std::strcmp(argv[i], "-ao") == 0) {
					if (i + 3 >= argc) {
						glow::error() << "No enough arguments: -ao <w> <h> <spp>";
						return -1;
					}

					aoWidth = std::atoi(argv[i + 1]);
					aoHeight = std::atoi(argv[i + 2]);
					aoSpp = std::atoi(argv[i + 3]);
					i += 4;
				}
				else if (std::strcmp(argv[i], "-irr") == 0) {
					if (i + 3 >= argc) {
						glow::error() << "No enough arguments: -irr <w> <h> <spp>";
						return -1;
					}

					irrWidth = std::atoi(argv[i + 1]);
					irrHeight = std::atoi(argv[i + 2]);
					irrSpp = std::atoi(argv[i + 3]);
					i += 4;
				}
				else if (std::strcmp(argv[i], "-light") == 0) {
					if (i + 1 >= argc) {
						glow::error() << "No enough arguments: -light <strength>";
						return -1;
					}

					lightStrength = std::atof(argv[i + 1]);
					i += 2;
				}
				else {
					glow::error() << "Unknown argument " << argv[i];
				}
			}
		}
		else {
			lmPath = arg;

			if (argc >= 4) {
				pdPath = std::string(argv[3]);
			}
		}
	}

	if (!outputPath.empty()) {
		PathTracer pathTracer;
		Scene scene;
		scene.loadFromGltf(gltfPath);
		scene.getSun().power = lightStrength;
		scene.buildPathTracerScene(pathTracer);
		IlluminationBaker illuminationBaker(pathTracer);

		std::vector<SharedImage> irradianceMaps;
		if (irrWidth > 0 && irrHeight > 0 && irrSpp > 0) {
			for (std::size_t i = 0; i < scene.getPrimitives().size(); ++i) {
				glow::info() << "Baking irradiance map " << i + 1 << " of " << scene.getPrimitives().size();
				auto lightMapImage = illuminationBaker.bakeIrradiance(scene.getPrimitives()[i], irrWidth, irrHeight, irrSpp);
				irradianceMaps.push_back(lightMapImage);
			}
		}

		std::vector<SharedImage> aoMaps;
		if (aoWidth > 0 && aoHeight > 0 && aoSpp > 0) {
			for (std::size_t i = 0; i < scene.getPrimitives().size(); ++i) {
				glow::info() << "Baking ambient occlusion map " << i + 1 << " of " << scene.getPrimitives().size();
				auto aoImage = illuminationBaker.bakeAmbientOcclusion(scene.getPrimitives()[i], aoWidth, aoHeight, aoSpp, 0.15f);
				aoMaps.push_back(aoImage);
			}
		}

		writeLightMapToFile(outputPath, irradianceMaps, aoMaps);
		return 0;
	}
	else {
		BakedGIApp sample(gltfPath, lmPath, pdPath);
		return sample.run(argc, argv); // automatically sets up GLOW and GLFW and everything
	}
}
