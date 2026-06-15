#pragma once

namespace Ecs {
	enum CameraPerspectiveType {
		Perspective,
		Orthographic
	};

	struct CameraComponent {
		float field_of_view;
		CameraPerspectiveType type;

	  public:
		CameraComponent(CameraPerspectiveType type, float field_of_view) : type(type), field_of_view(field_of_view) {}
	};

}
