#version 460 core

layout (location = 0) in ivec2 position;
layout (location = 1) in uvec3 col;
layout (location = 2) in uvec2 texpos;
layout (location = 3) in uvec2 texpage;

layout (location = 0) out vec3 color;

uniform uvec2 offset;

void main() {
    color = vec3(
        float(col.r) / 255.0,
        float(col.g) / 255.0,
        float(col.b) / 255.0
    );
    
    vec2 pos = position; // + offset;

    float x = float(pos.x / 512.0) - 1.0;
    float y = 1 - float(pos.y / 256.0);
    
    gl_Position = vec4(x, y, 0.0, 1.0);
}
