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

// axis aligned box centered at the origin, with size boxSize
vec2 boxIntersection( in vec3 ro, in vec3 rd, vec3 boxSize, out vec3 outNormal ) 
{
    vec3 m = 1.0/rd; // can precompute if traversing a set of aligned boxes
    vec3 n = m*ro;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(m)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0) return vec2(-1.0); // no intersection
    outNormal = -sign(rd)*step(t1.yzx,t1.xyz)*step(t1.zxy,t1.xyz);
    return vec2( tN, tF );
}

// cylinder defined by extremes pa and pb, and radious ra
vec4 cylIntersect( in vec3 ro, in vec3 rd, in vec3 pa, in vec3 pb, float ra )
{
    vec3 ca = pb-pa;
    vec3 oc = ro-pa;
    float caca = dot(ca,ca);
    float card = dot(ca,rd);
    float caoc = dot(ca,oc);
    float a = caca - card*card;
    float b = caca*dot( oc, rd) - caoc*card;
    float c = caca*dot( oc, oc) - caoc*caoc - ra*ra*caca;
    float h = b*b - a*c;
    if( h<0.0 ) return vec4(-1.0); //no intersection
    h = sqrt(h);
    float t = (-b-h)/a;
    // body
    float y = caoc + t*card;
    if( y>0.0 && y<caca ) return vec4( t, (oc+t*rd-ca*y/caca)/ra );
    // caps
    t = (((y<0.0)?0.0:caca) - caoc)/card;
    if( abs(b+a*t)<h ) return vec4( t, ca*sign(y)/caca );
    return vec4(-1.0); //no intersection
}

bool drawArrow(vec3 origin, vec3 dir, vec3 arrowStart, vec3 arrowEnd){
    float tcylinder = cylIntersect(origin, dir, arrowStart, arrowEnd, 0.05).x;
    return tcylinder > 0;
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

    int texSize = 8;
    vec3 lightPos = vec3(10, 0, 0);
    vec4 ambient = vec4(0.1, 0.1, 0.1, 1);
    if(drawArrow(origin, dir, vec3(0, 0, 0), vec3(1, 0, 0))) {
       out_color = vec4(1, 0, 0, 1);
       return;
    }
    if(drawArrow(origin, dir, vec3(0, 0, 0), vec3(0, 1, 0))) {
       out_color = vec4(0, 1, 0, 1);
        return;
    }
    if(drawArrow(origin, dir, vec3(0, 0, 0), vec3(0, 0, 1))) {
       out_color = vec4(0, 0, 1, 1);
       return;
    }
   

    vec3 boxNormal;
    float tn = boxIntersection(origin, dir, vec3(texSize), boxNormal).x;
    if(tn > 0)
    {
        //out_color = texture(myTexture, vec3(in_vertexUV, z));

        vec3 point = origin + dir * tn;
        point += texSize / 2;
        point /= texSize;
        point.y = 1 - point.y;

        /*
        */
        
        out_color = texture(myTexture, point);

    }
    else
    {
        out_color = vec4(0, 0, 0, 1);
    }
    
    vec3 spherePos = vec3(0, 0, 0);
    vec4 sphereColor = vec4(0, 1, 0, 1);


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