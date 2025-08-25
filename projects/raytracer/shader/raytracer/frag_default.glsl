#version 330

in vec2 uv;
out vec4 FragColor;

uniform float time;
uniform int sample_count;
uniform int bounce_count;

uniform mat4x4 camera_inv_proj;
uniform mat4x4 camera_to_world;
uniform vec3 camera_position;

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

// https://www.shadertoy.com/view/fdS3zw
vec3 cosineSampleHemisphere(vec3 n, inout uint seed) {
    vec2 u = vec2(RandomValue(seed), RandomValue(seed));

    float r = sqrt(u.x);
    float theta = 2.0 * 3.141529 * u.y;
 
    vec3  B = normalize( cross( n, vec3(0.0,1.0,1.0) ) );
	vec3  T = cross( B, n );
    
    return normalize(r * sin(theta) * B + sqrt(1.0 - u.x) * n + r * cos(theta) * T);
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
    int base_index = mesh_index * 5;
    MeshMetadata meta;
    
    vec4 col0 = texelFetch(mesh_data, base_index);
    vec4 col1 = texelFetch(mesh_data, base_index + 1);
    vec4 col2 = texelFetch(mesh_data, base_index + 2);
    vec4 col3 = texelFetch(mesh_data, base_index + 3);

    meta.matrix = mat4(col0, col1, col2, col3);
    meta.triangles = texelFetch(mesh_data, base_index + 4).r;
    
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
TracerRayHitInfo ray_intersects_triangle(vec3 ray_origin, vec3 ray_direction, vec3 p1, vec3 p2, vec3 p3) {
    TracerRayHitInfo ray_info;
    ray_info.hit = false;
    ray_info.intersect_distance = 0;

    vec3 edge1 = p2 - p1;
    vec3 edge2 = p3 - p1;
    vec3 ray_cross_e2 = cross(ray_direction, edge2);
    float det = dot(edge1, ray_cross_e2);

    if (det > -EPSILON && det < EPSILON)
        return ray_info;   // This ray is parallel to this triangle.

    float inv_det = 1.0 / det;
    vec3 s = ray_origin - p1;
    float u = inv_det * dot(s, ray_cross_e2);

    if ((u < 0 && abs(u) > EPSILON) || (u > 1 && abs(u-1) > EPSILON))
        return ray_info;

    vec3 s_cross_e1 = cross(s, edge1);
    float v = inv_det * dot(ray_direction, s_cross_e1);

    if ((v < 0 && abs(v) > EPSILON) || (u + v > 1 && abs(u + v - 1) > EPSILON))
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

RayPixelData TraceRay(Ray ray) {
    RayPixelData data;
    data.hit = false;
    data.color = vec3(0.0, 0.0, 0.0);

    float min_distance = 1e9;
    for (int i = 0; i < sphere_texture_count; i++) {
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
    for (int i = 0; mesh_count > i; i++) {
        MeshMetadata mesh = get_mesh_metadata(i);

        for (int t = 0; mesh.triangles > t; t++) {
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
                data.color = vec3(1.0, 0.0, 0.0);
                data.emission = 0;
                data.normal = cross(triangle.p2 - triangle.p1, triangle.p3 - triangle.p1);
                data.position = ray.origin + ray.direction * ray_test.intersect_distance;
            }
        }
    }
    
    return data;
}

// Main

void main() {
    vec3 pixel_direction = get_pixel_world_direction(uv);

    Ray ray;
    ray.origin = camera_position;
    ray.direction = pixel_direction;

    vec3 SKY_COLOR = vec3(0.0, 0.0, 0.0);
    RayPixelData test = TraceRay(ray);

    if (!test.hit) {
        discard;
    }

    vec3 first_test_normal = test.normal;
    vec3 incoming_light = test.color.rgb * test.emission * test.emission;
    vec3 output_color = test.color.rgb;
    uint random_seed = uint(uv.x * 473284784) + uint(uv.y * 173284784); // + uint(time);

    ray.origin = test.position + first_test_normal * 0.001;

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
