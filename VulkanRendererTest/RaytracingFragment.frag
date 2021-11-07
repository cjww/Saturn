#version 450

layout(location = 0) in vec2 in_vertexUV;
layout(location = 1) in flat uint in_vertexIndex;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler3D myTexture;

layout(set = 1, binding = 0) uniform Variables {
    mat4 mat;
    vec3 size;
    float time;
} var;

float sphereIntersect(vec3 ray_origin, vec3 ray_dir, vec3 center, float radius) {
    vec3 a = center - ray_origin;
    float tca = dot(a, ray_dir);
    if (tca < 0) return -1;
    float d2 = dot(a, a) - tca * tca;
    if(d2 > radius) return -1;
    float thc = sqrt(radius - d2);
    return tca - thc;
}

void main() {

    float z = (sin(var.time) + 1) * 0.5;
    
    float aspectRatio = var.size.x / var.size.y; 

    float fov = radians(45);
    float fov_factor = tan(fov * 0.5);

    vec3 fragmentPos = vec3(
        (2 * in_vertexUV.x - 1) * aspectRatio * fov_factor,
        (1 - 2 * in_vertexUV.y) * fov_factor,
        1); 

    vec3 origin = vec3(0, 0, 0);


    fragmentPos = (var.mat * vec4(fragmentPos, 1)).xyz;
    origin = (var.mat * vec4(origin, 1)).xyz;

    vec3 dir = fragmentPos - origin;
    dir = normalize(dir);

    out_color = texture(myTexture, vec3(in_vertexUV, z));

    vec3 spherePos = vec3(0, 0, 2 + z);
    vec4 sphereColor = vec4(1, 0, 0, 1);
    vec3 lightPos = vec3(z, 0, 0);

    vec4 ambient = vec4(0.1, 0.1, 0.1, 1);

    float t = sphereIntersect(origin, dir, spherePos, 0.5);
    if(t > 0)
    {
        vec3 pointOnSphere = origin + dir * t;
        vec3 sphereNormal = normalize(pointOnSphere - spherePos);
        vec3 lightDir = normalize(lightPos - pointOnSphere);
        float diff = dot(lightDir, sphereNormal);
        
        out_color = ambient + sphereColor * diff;
    }


}