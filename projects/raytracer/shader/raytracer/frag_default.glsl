#version 330

in vec2 uv;
out vec4 FragColor;

uniform float time;
uniform int sample_count;
uniform int bounce_count;

uniform mat4x4 camera_inv_proj;
uniform mat4x4 camera_to_world;
uniform vec3 camera_position;

uniform int use_debug;

// Object Data --------------------------------------------------------------------------------------------------------------------------------------------------------

uniform samplerBuffer mesh_triangle_data;
uniform samplerBuffer mesh_data;
uniform int mesh_count;

uniform samplerBuffer spheres_texture;
uniform int sphere_texture_count;

// Random --------------------------------------------------------------------------------------------------------------------------------------------------------
// PCG (permuted congruential generator). Thanks to:
// www.pcg-random.org and www.shadertoy.com/view/XlGcRh

uint NextRandom(inout uint state) {
    state = state * uint(747796405) + uint(2891336453);
    uint result = ((state >> ((state >> (28)) + uint(4))) ^ state) * uint(277803737);
    result = (result >> 22) ^ result;
    return result;
}

float RandomValue(inout uint state) {
    return NextRandom(state) / 4294967295.0; // 2^32 - 1
}

vec2 RandomVec2(inout uint state) {
    return vec2(RandomValue(state), RandomValue(state));
}

vec3 cosineSampleHemisphere(vec3 n, inout uint seed) {
    vec2 u = RandomVec2(seed);
    
    float cos_theta = sqrt(u.x);
    float sin_theta = sqrt(1.0 - u.x);
    float phi = 2.0 * 3.14159265359 * u.y;
    
    vec3 w = n;
    vec3 u_vec = normalize(cross((abs(w.x) > 0.1 ? vec3(0,1,0) : vec3(1,0,0)), w));
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
    vec4 data0 = texelFetch(spheres_texture, base_index);
    vec4 data1 = texelFetch(spheres_texture, base_index + 1);
    
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
    mat4 inv_matrix;
    vec3 color;
    vec3 box_min;
    vec3 box_max;
    float emissive;
    float triangles;
};

MeshTriangle get_mesh_triangle(int index) {
    int base_index = index * 3;

    MeshTriangle triangle;
    triangle.p1 = texelFetch(mesh_triangle_data, base_index).xyz;
    triangle.p2 = texelFetch(mesh_triangle_data, base_index + 1).xyz;
    triangle.p3 = texelFetch(mesh_triangle_data, base_index + 2).xyz;

    return triangle;
}

MeshMetadata get_mesh_metadata(int mesh_index) {
    int base_index = mesh_index * 12;
    MeshMetadata meta;
    
    vec4 col0 = texelFetch(mesh_data, base_index);
    vec4 col1 = texelFetch(mesh_data, base_index + 1);
    vec4 col2 = texelFetch(mesh_data, base_index + 2);
    vec4 col3 = texelFetch(mesh_data, base_index + 3);
    meta.matrix = mat4(col0, col1, col2, col3);

    col0 = texelFetch(mesh_data, base_index + 4);
    col1 = texelFetch(mesh_data, base_index + 5);
    col2 = texelFetch(mesh_data, base_index + 6);
    col3 = texelFetch(mesh_data, base_index + 7);
    meta.inv_matrix = mat4(col0, col1, col2, col3);

    meta.color = texelFetch(mesh_data, base_index + 8).xyz;
    meta.box_min = texelFetch(mesh_data, base_index + 9).xyz;
    meta.box_max = texelFetch(mesh_data, base_index + 10).xyz;

    meta.triangles = texelFetch(mesh_data, base_index + 11).r;
    meta.emissive = texelFetch(mesh_data, base_index + 11).g;
    
    return meta;
}

// Camera Data --------------------------------------------------------------------------------------------------------------------------------------------------------

vec3 get_pixel_world_direction(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0; // convert uv to clip space (-1, 1)
    vec4 clip_space_position = vec4(-ndc.xy, 0.0, -1.0); // clip position is camera clip position
    vec4 view_position = camera_inv_proj * clip_space_position; // transform it from clip space to camera space or something
    view_position /= view_position.w;

    vec3 view_direction = normalize(view_position.xyz / view_position.w); // view_direction is the direction from the camera to the pixel in view space
    return normalize(mat3x3(camera_to_world) * view_direction); // unity_CameraInvView is the camera-to-world matrix
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
TracerRayHitInfo ray_intersects_triangle(
    vec3 origin, vec3 direction, 
    vec3 v0, vec3 v1, vec3 v2) {
    
    TracerRayHitInfo info;
    info.hit = false;
    
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(direction, edge2);
    float a = dot(edge1, h);
    
    if (abs(a) < 0.000001) return info; // Ray parallel to triangle
    
    float f = 1.0 / a;
    vec3 s = origin - v0;
    float u = f * dot(s, h);
    
    if (u < 0.0 || u > 1.0) return info;
    
    vec3 q = cross(s, edge1);
    float v = f * dot(direction, q);
    
    if (v < 0.0 || u + v > 1.0) return info;
    
    float t = f * dot(edge2, q);
    if (t > 0.000001) {
        info.hit = true;
        info.intersect_distance = t;
    }
    
    return info;
}

bool ray_intersects_aabb_box(vec3 ray_origin, vec3 ray_direction, vec3 pos_min, vec3 pos_max) {
    vec3 inv_dir = 1.0 / ray_direction;

    vec3 t0 = (pos_min - ray_origin) * inv_dir; 
    vec3 t1 = (pos_max - ray_origin) * inv_dir;

    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float fmin = max(tmin.x, max(tmin.y, tmin.z));
    float fmax = min(tmax.x, min(tmax.y, tmax.z));

    return fmax > fmin && fmin > 0;
}

// quadratic formula expression of x^2 + y^2 + z^2 = r^2 transformed into ||P+Dx||^2 = R^2
TracerRayHitInfo RaySphereIntersect(vec3 sphere_center, float sphereRadius, Ray ray) {
    TracerRayHitInfo ray_info;
    ray_info.hit = false;
    ray_info.intersect_distance = 0;

    vec3 origin = ray.origin - sphere_center;
    vec3 rayOrigin = origin;
    vec3 rayDirection = -ray.direction;

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

vec3 mul_mat4_vec3(mat4 m, vec3 v, float s) {
    return (m * vec4(v, s)).xyz;
}

RayPixelData TraceRay(Ray ray, inout int tests) {
    RayPixelData data;
    data.hit = false;
    data.color = vec3(0.0, 0.0, 0.0);

    float min_distance = 1e9;
    for (int i = 0; i < sphere_texture_count; i++) {
        Sphere sphere = getSphereAtIndex(i);
        TracerRayHitInfo test = RaySphereIntersect(sphere.center, sphere.radius, ray); 
        tests += 1;
        
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
    for (int i = 0; mesh_count > i; i++) {
        MeshMetadata mesh = get_mesh_metadata(i);

        if (!ray_intersects_aabb_box(ray.origin, ray.direction, mesh.box_min, mesh.box_max)) {
            global_mesh_triangle_index += int(mesh.triangles);
            continue;
        }

        mat4 inv_matrix = mesh.inv_matrix;
        vec3 origin = mul_mat4_vec3(inv_matrix, ray.origin, 1.0);
        vec3 direction = normalize(mul_mat4_vec3(inv_matrix, ray.direction, 0.0));

        for (int t = 0; mesh.triangles > t; t++) {
            MeshTriangle triangle = get_mesh_triangle(global_mesh_triangle_index);
            global_mesh_triangle_index += 1;
            tests += 1;

            // transform triangles to world space
            TracerRayHitInfo ray_test = ray_intersects_triangle(origin, direction, triangle.p1, triangle.p2, triangle.p3);
            vec3 world_space_position = mul_mat4_vec3(mesh.matrix, origin + direction * ray_test.intersect_distance, 1.0);
            float world_space_distance = length(world_space_position - ray.origin);

            if (ray_test.hit && min_distance > world_space_distance) {
                min_distance = world_space_distance;
                
                data.hit = true;
                data.color = mesh.color;
                data.emission = mesh.emissive;
                data.normal = mul_mat4_vec3(mesh.matrix, cross(triangle.p2 - triangle.p1, triangle.p3 - triangle.p1), 0.0);
                data.position = world_space_position;
            }
        }
    }
    
    return data;
}

float hash13(vec3 p3) {
    p3 = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec3 pixel_direction = get_pixel_world_direction(uv);

    Ray ray;
    ray.origin = camera_position;
    ray.direction = pixel_direction;

    vec3 SKY_COLOR = vec3(0.0, 0.0, 0.0);
    vec3 incoming_light = vec3(0, 0, 0);

    float tests = 0;
    bool hit_sky = false;

    for (int i = 0; i < sample_count; i++) {
        ray.origin = camera_position;
        ray.direction = pixel_direction;
        vec3 output_color = vec3(1, 1, 1);

        float blue_noise = hash13(vec3(uv * 1000.0, float(i) + bounce_count + time));
        uint random_seed = uint(blue_noise * 4294967295.0) + uint(i * 12345); 

        for (int j = 0; j < bounce_count; j++) {
            RayPixelData test = TraceRay(ray, tests);

            if (test.hit) {
                ray.origin = test.position + normalize(test.normal) * 0.0001;            
                ray.direction = cosineSampleHemisphere(normalize(test.normal), random_seed);            
        
                incoming_light += test.emission * test.color * output_color.rgb;
                output_color *= test.color.rgb;
            } else {
                incoming_light += SKY_COLOR;

                if (j == 0) hit_sky = true;
                break;
            }

            if (dot(output_color, vec3(1, 1, 1)) < 1.0) break;
        }

        if (hit_sky == true) {
            break;
        }
    }

    vec3 final_color = incoming_light / sample_count;

    // aces
    final_color = (final_color * (2.51 * final_color + 0.03)) / 
                  (final_color * (2.43 * final_color + 0.59) + 0.14);

    tests /= 2000.0;

    if (use_debug == 1) {
        final_color = vec3(tests, tests, tests);

        if (tests > 1.0) {
            final_color = vec3(1, 0, 0);
        }
    }


    FragColor = vec4(final_color, 1.0);
}
