#version 330

in vec2 vertex_position;
in vec2 vertex_texcoord;
in vec4 vertex_color;

uniform sampler2D sampler_texture;
uniform float     using_texture   = 0;
uniform float     elapsed_time;

out vec4 output_color;
void main() {
    vec2 sample_coord = vertex_texcoord;

    sample_coord.x += sin( sample_coord.x * 2*8*3.14159 + elapsed_time) / 100;
    sample_coord.y += cos(-sample_coord.y * 4*8*3.14159 + elapsed_time) / 100;

    vec4 sampled_texture_color = mix(vertex_color, texture(sampler_texture, sample_coord) * vertex_color, using_texture);
    output_color = vec4(sampled_texture_color) * vec4(1);
}
