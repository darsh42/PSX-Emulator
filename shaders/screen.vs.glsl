#version 460 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 col;
layout (location = 2) in uvec2 texpos;
layout (location = 3) in uvec2 texpage;

layout (location = 0) out vec3 color;

// uniform uvec2 offset;

void main() {
    // Convert color components from 0-255 range to 0.0-1.0 range
    // color = vec3(
    //     float(col.r) / 255.0,
    //     float(col.g) / 255.0,
    //     float(col.b) / 255.0
    // );
    
    // Apply offset to position
    // vec2 pos = vec2(position) + vec2(offset);
    // 
    // // Convert to normalized device coordinates (NDC)
    // float x = (float(pos.x) / 512.0) * 2.0 - 1.0;
    // float y = 1.0 - (float(pos.y) / 256.0) * 2.0;
    
    color = col;
    gl_Position = vec4(position, 0.0, 1.0);
}
