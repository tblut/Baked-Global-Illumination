# Material Shading Library

## General considerations

* Diffuse Roughness != Specular Roughness
   * especially normal maps for diffuse and specular should be different
   * Idea: use higher LoD for diffuse
* Diffuse gets 'left-over' light from Specular
* Specular Color is F0 term in Fresnel Term (at 0°)
* Normal Distribution Function (NDF) incorporates anisotropy
* Always use Smith Function for Geometry term

## Material Types

### Metals (conductors)

* Have no diffuse color (total absorption)

### Non-Metals (dielectrics, insulators)

* Have achromatic specular color
