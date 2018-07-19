## What do I see here?

The main goal of this project is to precompute diffuse and specular global illumination. The lighting model used here is the one proposed by Epic Games, using GGX for specular and Lambert for diffuse.
Diffuse indirect lighting is precomputed using an unidirection path tracer that supports importance sampling for GGX and Lambert.
The resulting irradiance is stored in a lightmap. Parallax corrected reflection probes are used for specular and glossy reflections. They are manually placed using an in-app user interface.
To determine which pixel should use which probes, a "visibility" voxel grid is precomputed that hold the closest three probes per voxel. The voxels can then be accesed in the object shader to fetch
the correct three probes for interpolation. The interpolated probe sample is then used for the image-based lighting part of Epic Games' shading model.

## Setup

### Windows

Set the correct path to Embree 3.1 and add the bin/ directory in the Embree installation directory to the PATH variable.

### Linux

* To build the project open a terminal and run "cmake CMakeLists.txt && make -j".
* Before starting the application in any way, Embree must be added to the path with "source <embree_dir>/embree_vars.sh"
* To start the application navigate to the bin directory and run, for example, "./BakedGI ./models/test2.glb ./textures/test2.lm ./textures/test2.pd".
* To bake a new light map run "./BakedGI ./models/test2.glb -bake somename.lm -irr <width> <height> <samples_per_pixel>".

## Library licenses

* tiny_gltf.h : MIT license
* json.hpp : Copyright (c) 2013-2017 Niels Lohmann. MIT license.
* base64 : Copyright (C) 2004-2008 René Nyffenegger
* stb_image.h : v2.08 - public domain image loader - [Github link](https://github.com/nothings/stb/blob/master/stb_image.h)
* stb_image_write.h : v1.09 - public domain image writer - [Github link](https://github.com/nothings/stb/blob/master/stb_image_write.h)

## Textures

All textures are taken from [textures.com](https://www.textures.com/)
