// I got tired of my shader working terribly and had AI generate this because i'm tired of working on this. I will improve the old one by hand soon
precision highp float;
varying vec2 uv;

uniform float time;
uniform int sample_count;
uniform int bounce_count;

uniform mat4 camera_inv_proj;
uniform mat4 camera_to_world;
uniform vec3 camera_position;

// Object Data --------------------------------------------------------------------------------------------------------------------------------------------------------

uniform sampler2D mesh_triangle_data;
uniform sampler2D mesh_data;
uniform int mesh_count;

uniform sampler2D spheres_texture;
uniform int sphere_texture_count;

// TBO replacement helpers for WebGL compatibility ----------------------------------------------------------------------------------------
// These functions convert buffer indices to UV coordinates for texture2D lookups
// Assumes textures are 1D data packed into 2D textures with width = 512

float getTextureWidth() { return 512.0; } // Fixed texture width for data packing

vec2 indexToUV(int index) {
    float width = getTextureWidth();
    float x = mod(float(index), width);
    float y = floor(float(index) / width);
    return (vec2(x, y) + 0.5) / vec2(width, width); // Add 0.5 for pixel center sampling
}

vec4 texelFetchEmulated(sampler2D tex, int index) {
    return texture2D(tex, indexToUV(index));
}

// Random --------------------------------------------------------------------------------------------------------------------------------------------------------
// PCG (permuted congruential generator). Thanks to:
// www.pcg-random.org and www.shadertoy.com/view/XlGcRh

// Simple linear congruential generator for WebGL compatibility
float NextRandom(inout float state) {
    state = fract(sin(state * 12.9898) * 43758.5453);
    return state;
}

float RandomValue(inout float state) {
    return NextRandom(state);
}

vec2 RandomVec2(inout float state) {
    return vec2(RandomValue(state), RandomValue(state));
}

vec3 RandomOnUnitSphere(inout float state) {
    vec2 u = RandomVec2(state);
    float z = 1.0 - 2.0 * u.x;
    float r = sqrt(max(0.0, 1.0 - z * z));
    float phi = 2.0 * 3.14159265359 * u.y;
    return vec3(r * cos(phi), r * sin(phi), z);
}

vec3 cosineSampleHemisphere(vec3 n, inout float seed) {
    vec2 u = RandomVec2(seed);
    
    float cos_theta = sqrt(u.x);
    float sin_theta = sqrt(1.0 - u.x);
    float phi = 2.0 * 3.14159265359 * u.y;
    
    vec3 w = n;
    vec3 u_vec = normalize(cross((abs(w.x) > 0.1 ? vec3(0.0,1.0,0.0) : vec3(1.0,0.0,0.0)), w));
    vec3 v = cross(w, u_vec);
    
    return normalize(u_vec * cos(phi) * sin_theta + v * sin(phi) * sin_theta + w * cos_theta);
}

vec3 uniformSampleHemisphere(vec3 n, inout float seed) {
    vec2 u = RandomVec2(seed);
    
    float cos_theta = u.x;
    float sin_theta = sqrt(max(0.0, 1.0 - cos_theta * cos_theta));
    float phi = 2.0 * 3.14159265359 * u.y;
    
    vec3 w = n;
    vec3 u_vec = normalize(cross((abs(w.x) > 0.1 ? vec3(0.0,1.0,0.0) : vec3(1.0,0.0,0.0)), w));
    vec3 v = cross(w, u_vec);
    
    return normalize(u_vec * cos(phi) * sin_theta + v * sin(phi) * sin_theta + w * cos_theta);
}

// Sphere Data --------------------------------------------------------------------------------------------------------------------------------------------------------

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    float emission;
};

// Each sphere takes 4 texels (vec3+vec3+float+bool = 12+12+4+4 = 32 bytes)
// 32 bytes / 16 bytes per texel = 2 texels per sphere
Sphere getSphereAtIndex(int index) {
    int base_index = index * 2;
    vec4 data0 = texelFetchEmulated(spheres_texture, base_index);
    vec4 data1 = texelFetchEmulated(spheres_texture, base_index + 1);
    
    Sphere sphere;
    sphere.center = data0.xyz;
    sphere.radius = data0.w;
    sphere.color = data1.xyz;
    sphere.emission = data1.w;
    
    return sphere;
}

// Triangle Data --------------------------------------------------------------------------------------------------------------------------------------------------------

struct MeshTriangle {
    vec3 p1;
    vec3 p2;
    vec3 p3;
};

struct MeshMetadata {
    mat4 matrix;
    vec3 color;
    float emissive;
    float triangles;
};

MeshTriangle get_mesh_triangle(int index) {
    int base_index = index * 3;

    MeshTriangle triangle;
    triangle.p1 = texelFetchEmulated(mesh_triangle_data, base_index).xyz;
    triangle.p2 = texelFetchEmulated(mesh_triangle_data, base_index + 1).xyz;
    triangle.p3 = texelFetchEmulated(mesh_triangle_data, base_index + 2).xyz;

    return triangle;
}

MeshMetadata get_mesh_metadata(int mesh_index) {
    int base_index = mesh_index * 6;
    MeshMetadata meta;
    
    vec4 col0 = texelFetchEmulated(mesh_data, base_index);
    vec4 col1 = texelFetchEmulated(mesh_data, base_index + 1);
    vec4 col2 = texelFetchEmulated(mesh_data, base_index + 2);
    vec4 col3 = texelFetchEmulated(mesh_data, base_index + 3);

    meta.matrix = mat4(col0, col1, col2, col3);
    meta.color = texelFetchEmulated(mesh_data, base_index + 4).xyz;
    meta.triangles = texelFetchEmulated(mesh_data, base_index + 4).w;
    meta.emissive = texelFetchEmulated(mesh_data, base_index + 5).r;
    
    return meta;
}

// Camera Data --------------------------------------------------------------------------------------------------------------------------------------------------------

vec3 get_pixel_world_direction(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0; // convert uv to clip space (-1, 1)
    vec4 clip_space_position = vec4(-ndc.xy, 0.0, -1.0); // clip position is camera clip position
    vec4 view_position = camera_inv_proj * clip_space_position; // transform it from clip space to camera space or something
    view_position /= view_position.w;

    vec3 view_direction = normalize(view_position.xyz / view_position.w); // view_direction is the direction from the camera to the pixel in view space
    return normalize(mat3(camera_to_world) * view_direction); // unity_CameraInvView is the camera-to-world matrix
}

// Ray Physics --------------------------------------------------------------------------------------------------------------------------------------------------------

struct TracerRayHitInfo { 
    float intersect_distance;
    vec3 intersect_normal;
    vec3 intersect_position;
    bool hit;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

// TODO: reimplement yourself
const float EPSILON = 1e-6;
TracerRayHitInfo ray_intersects_triangle(vec3 ray_origin, vec3 ray_direction, vec3 p1, vec3 p2, vec3 p3) {
    TracerRayHitInfo ray_info;
    ray_info.hit = false;
    ray_info.intersect_distance = 0.0;

    vec3 edge1 = p2 - p1;
    vec3 edge2 = p3 - p1;
    vec3 ray_cross_e2 = cross(ray_direction, edge2);
    float det = dot(edge1, ray_cross_e2);

    if (det > -EPSILON && det < EPSILON)
        return ray_info;   // This ray is parallel to this triangle.

    float inv_det = 1.0 / det;
    vec3 s = ray_origin - p1;
    float u = inv_det * dot(s, ray_cross_e2);

    if ((u < 0.0 && abs(u) > EPSILON) || (u > 1.0 && abs(u - 1.0) > EPSILON))
        return ray_info;

    vec3 s_cross_e1 = cross(s, edge1);
    float v = inv_det * dot(ray_direction, s_cross_e1);

    if ((v < 0.0 && abs(v) > EPSILON) || (u + v > 1.0 && abs(u + v - 1.0) > EPSILON))
        return ray_info;

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = inv_det * dot(edge2, s_cross_e1);

    if (t > EPSILON) // ray intersection
    {    
        ray_info.hit = true;
        ray_info.intersect_distance = t;
        return ray_info;
    } else // This means that there is a line intersection but not a ray intersection.

    return ray_info;
}


// quadratic formula expression of x^2 + y^2 + z^2 = r^2 transformed into ||P+Dx||^2 = R^2
TracerRayHitInfo RaySphereIntersect(vec3 sphere_center, float sphereRadius, Ray ray) {
    TracerRayHitInfo ray_info;
    ray_info.hit = false;
    ray_info.intersect_distance = 0.0;

    vec3 origin = ray.origin - sphere_center;
    vec3 rayOrigin = origin;
    vec3 rayDirection = -ray.direction;

    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(rayOrigin, rayDirection);
    float c = dot(rayOrigin, rayOrigin) - sphereRadius * sphereRadius;

    float delta = b * b - 4.0 * a * c;

    if (delta >= 0.0) {
        float t1 = (-b + sqrt(delta)) / (2.0 * a);
        float t2 = (-b - sqrt(delta)) / (2.0 * a);

        float t = min(t1, t2);
        if (t < 0.0) t = max(t1, t2);

        // avoids self intersect apparently
        if (t < 0.00001) {
            ray_info.hit = true;
            ray_info.intersect_position = ray.origin + ray.direction * t;
            ray_info.intersect_distance = -t;
            ray_info.intersect_normal = -normalize(ray_info.intersect_position - sphere_center);
        }
    }

    return ray_info;
}


// Ray Light ---------------------------------------------------------------------------------------------------------------------------------------------

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
    for (int i = 0; i < 32; i++) {
        if (i >= sphere_texture_count) break;
        Sphere sphere = getSphereAtIndex(i);
        TracerRayHitInfo test = RaySphereIntersect(sphere.center, sphere.radius, ray); 

        if (test.hit && min_distance > test.intersect_distance) {
            min_distance = test.intersect_distance;
            
            data.color = sphere.color.rgb;
            data.emission = sphere.emission;
            data.normal = test.intersect_normal;
            data.position = test.intersect_position;
            
            data.hit = true;
        }
    }

    int global_mesh_triangle_index = 0;
    for (int i = 0; i < 16; i++) {
        if (i >= mesh_count) break;
        MeshMetadata mesh = get_mesh_metadata(i);

        for (int t = 0; t < 1024; t++) {
            if (float(t) >= mesh.triangles) break;
            MeshTriangle triangle = get_mesh_triangle(global_mesh_triangle_index);
            global_mesh_triangle_index += 1;
            
            // transform triangles to world space
            triangle.p1 = (mesh.matrix * vec4(triangle.p1, 1.0)).xyz;
            triangle.p2 = (mesh.matrix * vec4(triangle.p2, 1.0)).xyz;
            triangle.p3 = (mesh.matrix * vec4(triangle.p3, 1.0)).xyz;

            TracerRayHitInfo ray_test = ray_intersects_triangle(ray.origin, ray.direction, triangle.p1, triangle.p2, triangle.p3);
            if (ray_test.hit && min_distance > ray_test.intersect_distance) {
                min_distance = ray_test.intersect_distance;
                
                data.hit = true;
                data.color = mesh.color;
                data.emission = mesh.emissive;
                data.normal = cross(triangle.p2 - triangle.p1, triangle.p3 - triangle.p1);
                data.position = ray.origin + ray.direction * ray_test.intersect_distance;
            }
        }
    }
    
    return data;
}

// Main

vec3 PathTrace(Ray initial_ray, inout float random_seed) {
    vec3 color = vec3(0.0);
    vec3 throughput = vec3(1.0);
    Ray ray = initial_ray;
    
    for (int bounce = 0; bounce < 16; bounce++) {
        if (bounce >= bounce_count) break;
        RayPixelData hit = TraceRay(ray);
        
        if (!hit.hit) {
            color += throughput * vec3(0.135, 0.206, 0.235); // Sky color
            break;
        }
        
        vec3 normal = normalize(hit.normal);
        vec3 hit_point = hit.position + normal * 0.001;
        
        // Add emission
        color += throughput * hit.color * hit.emission;
        
        // Russian roulette termination
        float max_component = max(throughput.r, max(throughput.g, throughput.b));
        if (bounce > 3) {
            if (RandomValue(random_seed) > max_component) {
                break;
            }
            throughput /= max_component;
        }
        
        // Sample next direction using cosine-weighted hemisphere sampling
        vec3 new_direction = cosineSampleHemisphere(normal, random_seed);
        
        // Update throughput with BRDF and cosine term
        // For Lambertian: BRDF = albedo/pi, pdf = cos_theta/pi
        // The cos_theta terms cancel out in Monte Carlo integration
        throughput *= hit.color;
        
        // Setup next ray
        ray.origin = hit_point;
        ray.direction = new_direction;
    }
    
    return color;
}

// Blue noise pattern for better sampling distribution
float hash13(vec3 p3) {
    p3  = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec3 pixel_direction = get_pixel_world_direction(uv);
    
    vec3 final_color = vec3(0.0);
    
    // Monte Carlo integration with blue noise
    for (int sample = 0; sample < 128; sample++) {
        if (sample >= sample_count) break;
        // Blue noise based seed for better sample distribution
        float blue_noise = hash13(vec3(uv * 1000.0, float(sample) + time));
        float random_seed = blue_noise + float(sample) * 0.12345;
        
        Ray ray;
        ray.origin = camera_position;
        ray.direction = pixel_direction;
        
        final_color += PathTrace(ray, random_seed);
    }
    
    // Average samples
    final_color /= float(sample_count);
    
    // Improved tone mapping (ACES approximation)
    final_color = (final_color * (2.51 * final_color + 0.03)) / 
                  (final_color * (2.43 * final_color + 0.59) + 0.14);
    
    // Clamp to prevent NaN/Inf values
    final_color = clamp(final_color, 0.0, 1.0);
    
    gl_FragColor = vec4(final_color, 1.0);
}
