#version 330

in vec2 uv;
out vec4 FragColor;

uniform float time;
uniform int sample_count;
uniform int bounce_count;

uniform mat4x4 camera_inv_proj;
uniform mat4x4 camera_to_world;
uniform vec3 camera_position;

// later idea for storing vertices:
// buffer for vertex positions
// buffer for objects
// buffer for object tris counts

uniform samplerBuffer mesh_triangle_data;
uniform samplerBuffer mesh_data;
uniform int mesh_count;

uniform samplerBuffer spheres_texture;
uniform int sphere_texture_count;

// PCG (permuted congruential generator). Thanks to:
// www.pcg-random.org and www.shadertoy.com/view/XlGcRh
uint NextRandom(inout uint state)
{
    state = state * uint(747796405) + uint(2891336453);
    uint result = ((state >> ((state >> (28)) + uint(4))) ^ state) * uint(277803737);
    result = (result >> 22) ^ result;
    return result;
}

float RandomValue(inout uint state)
{
    return NextRandom(state) / 4294967295.0; // 2^32 - 1
}

// https://www.shadertoy.com/view/fdS3zw
vec3 cosineSampleHemisphere(vec3 n, inout uint seed) {
    vec2 u = vec2(RandomValue(seed), RandomValue(seed));

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
    float emission;
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
    sphere.emission = data1.w;
    
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
        if (t < 0.00001) {
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
            data.emission = sphere.emission;
            data.normal = test.intersectNormal;
            data.position = test.intersectPosition;
            
            data.hit = true;
        }
    }
    
    return data;
}

void main() {
    vec3 pixel_direction = -GetPixelWorldDirection(uv);

    Ray ray;
    ray.origin = camera_position;
    ray.direction = pixel_direction;

    vec3 SKY_COLOR = vec3(0, 0, 0);
    RayPixelData test = TraceRay(ray);

    if (!test.hit) {
        discard;
    }

    vec3 first_test_normal = test.normal;
    vec3 incoming_light = test.color.rgb * test.emission * test.emission;
    vec3 output_color = test.color.rgb;
    uint random_seed = uint(uv.x * 473284784) + uint(uv.y * 173284784); // + uint(time);

    ray.origin = test.position + first_test_normal * 0.00001;

    for (int i = 0; i < sample_count; i++) {
        ray.direction = cosineSampleHemisphere(first_test_normal, random_seed);
        test = TraceRay(ray);

        if (test.hit) {
            output_color *= vec3(test.color.rgb);
            vec3 new_incoming_light = output_color * test.emission;
            
            Ray bounce_ray;
            bounce_ray.origin = ray.origin;
        
            for (int i = 0; i < bounce_count; i++) {
                bounce_ray.direction = cosineSampleHemisphere(ray.direction, random_seed);
                RayPixelData bounce_test = TraceRay(bounce_ray);

                if (bounce_test.hit) {
                    bounce_ray.origin = bounce_test.position + bounce_test.normal * 0.0001;
                    output_color *= bounce_test.color.rgb;
                    new_incoming_light += bounce_test.emission * output_color.rgb;
                }
            }

            incoming_light += new_incoming_light;
        } else {
            incoming_light += SKY_COLOR;
        }
    }

    FragColor = vec4(incoming_light / sample_count, 1.0);
}