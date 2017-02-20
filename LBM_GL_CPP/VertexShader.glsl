#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in float color;

out vec3 fColor;

uniform vec4 viewportMatrix;
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

vec3 unpackColor(float f)
{
    uint f2 = floatBitsToUint(f);

    uint r = (f2 & uint(0x000000FF));
    uint g = (f2 & uint(0x0000FF00)) >> 8;
    uint b = (f2 & uint(0x00FF0000)) >> 16;
    uint a = (f2 & uint(0xFF000000));

    float rf = float(r);
    float gf = float(g);
    float bf = float(b);
    vec3 color;
    color.x = rf/256.0;
    color.y = gf/256.0;
    color.z = bf/256.0;

    return color;
}

void main()
{
    
    gl_Position = projectionMatrix*modelMatrix*vec4(position, 1.f);

    vec3 unpackedColor = unpackColor(color);

    fColor.x = unpackedColor.x;
    fColor.y = unpackedColor.y;// color.y*sin((time + position.x + 1.f)*0.3f);
    fColor.z = unpackedColor.z;//color.z*sin((time + position.x + 1.f)*0.3f);
}