

#include "resource/image/image.h"
#include "resource/mesh/mesh.h"
#include <vector>

class EngineResource {};

class EngineMeshResource {
	MeshGeometry geometry;
};

namespace Resource {
	class Bank {
	  public:
		std::vector<MeshGeometry> meshes = std::vector<MeshGeometry>();
		std::vector<ImageData> images = std::vector<ImageData>();
	};
}