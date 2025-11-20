export module synodic.soul.tracer:camera;

import std;
import synodic.soul.core;
import :film;
import std;

// TODO: Replace glm types with module types
struct vec3 { float x, y, z; };
struct vec2 { float x, y; };

export class Camera {
public:
	Camera();
	~Camera();

	void OffsetOrientation(float x, float y);
	void UpdateVariables();

	float aspectRatio;
	vec3 position;
	vec3 forward;
	vec3 right;
	vec2 fieldOfView;
	Film film;

private:
	float aperture;
	float focalDistance;

	// VARIABLE PRECALC
	vec3 verticalAxis;
	vec3 yHelper;
	vec3 xHelper;
};

// Implementation
// TODO: Replace glm functions with module implementations
// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/gtx/rotate_vector.hpp>

inline Camera::Camera() :
	aspectRatio(0),
	position{0.0f, 0.0f, 0.0f},
	forward{0.0f, 0.0f, 1.0f},
	right{1.0f, 0.0f, 0.0f},
	fieldOfView{90.0f, 65.0f},
	aperture(0.02f),
	focalDistance(0.17f) {
}

inline Camera::~Camera() {
}

inline void Camera::UpdateVariables() {
	// TODO: Implement cross product, normalize, tan, radians when proper math types are available
	// verticalAxis = normalize(cross(right, forward));
	// yHelper = verticalAxis * tan(radians(-fieldOfView.y * 0.5f));
	// xHelper = right * tan(radians(fieldOfView.x * 0.5f));
}

inline void Camera::OffsetOrientation(float x, float y) {
	// TODO: Implement rotation when proper math types are available
	// right = normalize(rotateY(right, radians(x)));
	// forward = normalize(rotateY(forward, radians(x)));
	// forward = normalize(rotate(forward, radians(y), right));
}
