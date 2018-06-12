#include "LightMapBaker.hh"
#include "third-party/trianglepacker.hpp"
#include "Primitive.hh"
#include "PathTracer.hh"

LightMapBaker::LightMapBaker(const PathTracer& pathTracer) : pathTracer(&pathTracer) {

}

SharedImage LightMapBaker::bakeLightMap(const Primitive& primitive, int width, int height) {
	
}