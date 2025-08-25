#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform float time;

uniform mat4x4 camera_inv_proj;
uniform mat4x4 camera_to_world;
uniform vec3 camera_position;

vec3 GetPixelWorldDirection(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0; // convert uv to clip space (-1, 1)
    vec4 clipPos = vec4(-ndc.x, -ndc.y, 0, 1); // clip position is camera clip position
    vec4 viewPos = camera_inv_proj * clipPos; // transform it from clip space to camera space or something

    vec3 viewDir = normalize(viewPos.xyz / viewPos.w); // viewDir is the direction from the camera to the pixel in view space
    vec3 worldRayDirection = normalize(mat3x3(camera_to_world) * viewDir); // unity_CameraInvView is the camera-to-world matrix

    return worldRayDirection;
}

struct TracerRayHitInfo { 
    float Distance;
    vec3 Normal;
    bool Hit;
};

// quadratic formula expression of x^2 + y^2 + z^2 = r^2 transformed into ||P+Dx||^2 = R^2
TracerRayHitInfo RaySphereIntersect(vec3 SpherePos, float SphereRadius, vec3 RayOrigin, vec3 RayDir) {
    TracerRayHitInfo rayInfo;

    float a = dot(RayDir, RayDir);
    float b = 2 * dot(RayOrigin, RayDir);
    float c = dot(RayOrigin, RayOrigin) - SphereRadius * SphereRadius;

    float delta = b * b - 4 * a * c;

    if (delta >= 0) {
        float intersect1 = (-b + sqrt(delta)) / 2 * a;
        float intersect2 = (-b - sqrt(delta)) / 2 * a;
        float minIntersect = min(intersect1, intersect2);

        if (minIntersect < 0) {
            rayInfo.Hit = true;
            rayInfo.Distance = -minIntersect;
            rayInfo.Normal = normalize((RayDir * minIntersect) - SpherePos);
        }
    }

    return rayInfo;
}

void main() {
    vec3 worldDir = GetPixelWorldDirection(uv);

    TracerRayHitInfo test = RaySphereIntersect(vec3(0.0, 0.0, 0.0), 2.0, camera_position, worldDir); 
    if (test.Hit) {
        FragColor = vec4(test.Normal.x, test.Normal.y, test.Normal.z, 1.0);
        return;
    }

    FragColor = vec4(worldDir.r, worldDir.y, worldDir.z, 1.0);
}