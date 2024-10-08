#version 450 core

layout(location = 0) in vec3 fColor;
layout(location = 1) flat in uvec2 fTexpos;
layout(location = 2) flat in uvec2 fTexpage;
layout(location = 3) flat in uvec2 fClut;
layout(location = 4) flat in uint  fType;
layout(location = 5) flat in uint  fBlend;
layout(location = 6) flat in uint  fDepth;
layout(location = 7) flat in uint  fDrawtextures;
layout(location = 8) flat in uint  fSemi_transparent;

layout(location = 0) out vec4 frag_color;

layout(binding = 0) uniform sampler2D vram_texture;

#define NO_TEXTURE      0U
#define RAW_TEXTURE     1U
#define TEXTURE_BLEND   2U

vec4 vram_read(uint x, uint y) {
    return texelFetch(vram_texture, ivec2(int(x), int(y)), 0);
}

uint unpack_texel(vec4 texel) {
    uint i0 = uint(texel.a);

    texel *= ivec4(0x1f);
    uint i1 = uint(texel.b + 0.5);
    uint i2 = uint(texel.g + 0.5);
    uint i3 = uint(texel.r + 0.5);

    return (i0 << 15) | (i1 << 10) | (i2 << 5) | i3;
}

vec4 sample_texel(vec2 tc) {
    vec4 pixel = vec4(1.0, 0.0, 0.0, 1.0);
    float stride_divisor = 16.0;

    if (fDepth == 0u) {
        stride_divisor /= 4.0;
        int tc_x = int(fTexpage.x) + int(tc.x / stride_divisor);
        int tc_y = int(fTexpage.y) + int(tc.y);

        // Fetch texel from VRAM. Each 4-bits is an index into the CLUT
        uint texel = unpack_texel(vram_read(uint(tc_x), uint(tc_y)));

        // Fetch each pixel (located in the clut) in the texel in 4bit chunks
        uint clut_index = ((texel >> ((uint(tc.x) % 4u) * 4u)) & 0xfu);
        pixel = vram_read(uint(fClut.x + clut_index), uint(fClut.y));
    } else if (fDepth == 1u) {
        int tc_x = int(fTexpage.x) + int(tc.x / 2);
        int tc_y = int(fTexpage.y) + int(tc.y);
        
        if (fBlend == RAW_TEXTURE) {
            uint texel = unpack_texel(texelFetch(vram_texture, ivec2(ivec2(tc.x / 2, tc.y) + ivec2(fTexpage)), 0));

            uint clut_index = ((texel >> ((uint(tc.x) % 2u) * 8u)) & 0xFFu);
            pixel = vram_read(uint(fClut.x + clut_index), uint(fClut.y));
        }
    } else if (fDepth == 2u) {
        stride_divisor /= 16.0;
        int tc_x = int(fTexpage.x) + int(tc.x / stride_divisor);
        int tc_y = int(fTexpage.y) + int(tc.y);
        
        if (fBlend == RAW_TEXTURE) {
            pixel = vram_read(uint(tc_x), uint(tc_y));
        }
    }
    
    if (pixel == vec4(0))
        discard;

    return pixel;
}

void main() {
    // Texture drawing
    if (fDrawtextures == 1U) {
        frag_color = sample_texel(fTexpos);
        // Debug output
        if (frag_color == vec4(1.0, 0.0, 0.0, 1.0)) {
            frag_color = vec4(0.0, 1.0, 0.0, 1.0); // Indicate error in sampling
        }
    } else {
        // Triangle with Gouraud Shading
        frag_color = vec4(fColor, 1.0);
    }
}
