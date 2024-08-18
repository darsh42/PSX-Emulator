#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texpos;
layout(location = 3) in vec2 texpage;
layout(location = 4) in vec2 clutpos;
layout(location = 5) in uint type;
layout(location = 6) in uint blend;
layout(location = 7) in uint depth;
layout(location = 8) in uint draw_texture;
layout(location = 9) in uint semi_transparent;

out vec3 fColor;
flat out uvec2 fTexpos;
flat out uvec2 fTexpage;
flat out uvec2 fClut;
flat out uint  fType;
flat out uint  fBlend;
flat out uint  fDepth;
flat out uint  fDrawtextures;
flat out uint  fSemi_transparent;

vec2 vertex_norm(vec2 p) {
    return vec2(
        (p.x / 512.0) - 1.0,
        1.0 - (p.y / 256.0)
    );
}

void main() {
    gl_Position = vec4(vertex_norm(position), 0.0, 1.0);

    fColor = color;
    fTexpos = uvec2(texpos);
    fTexpage = uvec2(texpage);
    fClut = uvec2(clutpos);
    fType = type;
    fBlend = blend;
    fDepth = depth;
    fDrawtextures = draw_texture;
    fSemi_transparent = semi_transparent;
}
