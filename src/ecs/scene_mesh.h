#pragma once

namespace Ecs {
	// References a mesh that lives in the engine resource bank
	// (EngineInstance::resources.meshes) by its index. The renderer iterates every
	// entity that has this component, looks up the matching GPU buffers, and draws
	// it with the mesh pipeline.
	struct MeshComponent {
		int mesh_index = -1;

	  public:
		MeshComponent(int mesh_index) : mesh_index(mesh_index) {}
	};

}
