#version 330

in vec2 uv;
out vec4 FragColor;

uniform float time;

uniform mat4x4 camera_inv_proj;
uniform mat4x4 camera_to_world;
uniform vec3 camera_position;

// later idea for storing vertices:
// buffer for vertex positions
// buffer for objects
// buffer for object tris counts

uniform samplerBuffer spheres_texture;
uniform int sphere_texture_count;

struct Sphere {
    vec3 center;
    vec3 color;
    float radius;
    bool emissive;
};

Sphere getSphereAtIndex(int index) {
    // Each sphere takes 4 texels (vec3+vec3+float+bool = 12+12+4+4 = 32 bytes)
    // 32 bytes / 16 bytes per texel = 2 texels per sphere
    int baseIndex = index * 2;
    
    vec4 data0 = texelFetch(spheres_texture, baseIndex);
    vec4 data1 = texelFetch(spheres_texture, baseIndex + 1);
    
    Sphere sphere;
    sphere.center = data0.xyz;
    sphere.color = vec3(data0.w, data1.xy);
    sphere.radius = data1.z;
    sphere.emissive = (data1.w > 0.5);
    
    return sphere;
}

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

struct Ray {
    vec3 origin;
    vec3 direction;
};

// quadratic formula expression of x^2 + y^2 + z^2 = r^2 transformed into ||P+Dx||^2 = R^2
TracerRayHitInfo RaySphereIntersect(vec3 spherePosition, float sphereRadius, Ray ray) {
    TracerRayHitInfo rayInfo;
    rayInfo.Hit = false;
    rayInfo.Distance = -1;

    vec3 rayOrigin = ray.origin - spherePosition;
    vec3 rayDirection = ray.direction;

    float a = dot(rayDirection, rayDirection);
    float b = 2 * dot(rayOrigin, rayDirection);
    float c = dot(rayOrigin, rayOrigin) - sphereRadius * sphereRadius;

    float delta = b * b - 4 * a * c;

    if (delta >= 0) {
        float intersect1 = (-b + sqrt(delta)) / 2 * a;
        float intersect2 = (-b - sqrt(delta)) / 2 * a;
        float minIntersect = min(intersect1, intersect2);

        if (minIntersect < 0) {
            rayInfo.Hit = true;
            rayInfo.Distance = -minIntersect;
            rayInfo.Normal = normalize((rayDirection * minIntersect) - spherePosition);
        }
    }

    return rayInfo;
}

void main() {
    Ray ray;
    ray.origin = camera_position;
    ray.direction = GetPixelWorldDirection(uv);

    bool hasHit = false;
    float minDistance = 10000;
    for (int i = 0; i < sphere_texture_count; i++) {
        Sphere sphere = getSphereAtIndex(i);
        TracerRayHitInfo test = RaySphereIntersect(sphere.center, sphere.radius, ray); 

        if (test.Hit && minDistance > test.Distance) {
            minDistance = test.Distance;
            FragColor = vec4(test.Normal.x, test.Normal.y, test.Normal.z, 1.0);
            hasHit = true;
        }
    }

    if (hasHit) return;

    float a = 0.5 - ray.direction.y / 2;
    FragColor = vec4(a, a, a, 1.0);

    // FragColor = vec4(ray.direction.r, ray.direction.y, ray.direction.z, 1.0);
}