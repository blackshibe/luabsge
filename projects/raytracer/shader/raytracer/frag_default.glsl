#version 330

in vec2 uv;
out vec4 FragColor;

uniform float time;
uniform int bounces;

uniform mat4x4 camera_inv_proj;
uniform mat4x4 camera_to_world;
uniform vec3 camera_position;

// later idea for storing vertices:
// buffer for vertex positions
// buffer for objects
// buffer for object tris counts

uniform samplerBuffer spheres_texture;
uniform int sphere_texture_count;

float hash1(inout float seed) {
    return fract(sin(seed += 0.1)*43758.5453123);
}

vec2 hash2(inout float seed) {
    return fract(sin(vec2(seed+=0.1,seed+=0.1))*vec2(43758.5453123,22578.1459123));
}

// https://www.shadertoy.com/view/fdS3zw
vec3 cosineSampleHemisphere(vec3 n, inout float seed) {
    vec2 u = hash2(seed);

    float r = sqrt(u.x);
    float theta = 2.0 * 3.141529 * u.y;
 
    vec3  B = normalize( cross( n, vec3(0.0,1.0,1.0) ) );
	vec3  T = cross( B, n );
    
    return normalize(r * sin(theta) * B + sqrt(1.0 - u.x) * n + r * cos(theta) * T);
}

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    float emissive;
};

// Each sphere takes 4 texels (vec3+vec3+float+bool = 12+12+4+4 = 32 bytes)
// 32 bytes / 16 bytes per texel = 2 texels per sphere
Sphere getSphereAtIndex(int index) {
    int baseIndex = index * 2;
    vec4 data0 = texelFetch(spheres_texture, baseIndex);
    vec4 data1 = texelFetch(spheres_texture, baseIndex + 1);
    
    Sphere sphere;
    sphere.center = data0.xyz;
    sphere.radius = data0.w;
    sphere.color = data1.xyz;
    sphere.emissive = data1.w;
    
    return sphere;
}

vec3 GetPixelWorldDirection(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0; // convert uv to clip space (-1, 1)
    vec4 clipPos = vec4(-ndc.xy, 0.0, -1.0); // clip position is camera clip position
    vec4 viewPos = camera_inv_proj * clipPos; // transform it from clip space to camera space or something
    viewPos /= viewPos.w;

    vec3 viewDir = normalize(viewPos.xyz / viewPos.w); // viewDir is the direction from the camera to the pixel in view space
    vec3 worldRayDirection = normalize(mat3x3(camera_to_world) * viewDir); // unity_CameraInvView is the camera-to-world matrix

    return worldRayDirection;
}

struct TracerRayHitInfo { 
    float intersectDistance;
    vec3 intersectNormal;
    vec3 intersectPosition;
    bool hit;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

// quadratic formula expression of x^2 + y^2 + z^2 = r^2 transformed into ||P+Dx||^2 = R^2
TracerRayHitInfo RaySphereIntersect(vec3 sphere_center, float sphereRadius, Ray ray) {
    TracerRayHitInfo rayInfo;
    rayInfo.hit = false;
    rayInfo.intersectDistance = 0;

    vec3 origin = ray.origin - sphere_center;
    vec3 rayOrigin = origin;
    vec3 rayDirection = ray.direction;

    float a = dot(rayDirection, rayDirection);
    float b = 2 * dot(rayOrigin, rayDirection);
    float c = dot(rayOrigin, rayOrigin) - sphereRadius * sphereRadius;

    float delta = b * b - 4 * a * c;

    if (delta >= 0) {
        float t1 = (-b + sqrt(delta)) / (2 * a);
        float t2 = (-b - sqrt(delta)) / (2 * a);

        float t = min(t1, t2);
        if (t < 0.0) t = max(t1, t2);

        // avoids self intersect apparently
        if (t < 0.001) {
            rayInfo.hit = true;
            rayInfo.intersectPosition = ray.origin + ray.direction * t;
            rayInfo.intersectDistance = -t;
            rayInfo.intersectNormal = -normalize(rayInfo.intersectPosition - sphere_center);
        }
    }

    return rayInfo;
}

struct RayPixelData {
    bool hit;
    vec3 color;
    vec3 normal;
    vec3 position;
    float emission;
};

RayPixelData TraceRay(Ray ray) {
    RayPixelData data;
    data.hit = false;
    data.color = vec3(0.0, 0.0, 0.0);

    float min_distance = 1e9;
    for (int i = 0; i < sphere_texture_count; i++) {
        Sphere sphere = getSphereAtIndex(i);
        TracerRayHitInfo test = RaySphereIntersect(sphere.center, sphere.radius, ray); 

        if (test.hit && min_distance > test.intersectDistance) {
            min_distance = test.intersectDistance;
            
            data.color = sphere.color.rgb;
            data.emission = sphere.emissive;
            data.normal = test.intersectNormal;
            data.position = test.intersectPosition;
            
            data.hit = true;
        }
    }
    
    return data;
}

void main() {
    Ray ray;
    ray.origin = camera_position;
    ray.direction = -GetPixelWorldDirection(uv);

    vec4 color = vec4(0, 0, 0, 1);

    RayPixelData test = TraceRay(ray);
    if (!test.hit) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 normal = test.normal;
    float seed = uv.x + uv.y * 3.43121412313 + fract(1.12345314312);
    float emission = test.emission * bounces;

    color = vec4(test.color.rgb * bounces, 1.0);
    ray.origin = test.position + test.normal * 0.001;

    for (int done_bounces = 0; done_bounces < bounces; done_bounces++) {
        color += vec4(test.color.rgb, 1.0);
        emission += test.emission;
        ray.direction = cosineSampleHemisphere(normal, seed);

        test = TraceRay(ray);
    }

    FragColor = vec4((color.rgb / bounces) * (emission / bounces), 1.0);
}