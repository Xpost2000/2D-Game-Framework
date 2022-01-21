#version 330

in vec2 vertex_position;
in vec2 vertex_texcoord;
in vec4 vertex_color;

uniform sampler2D sampler_texture;
uniform float     using_texture   = 0;
uniform float     elapsed_time;

out vec4 output_color;
void main() {
    vec2 s = vertex_texcoord;
    // s.y *=;
    vec4 sampled_texture_color = mix(vertex_color, texture(sampler_texture, s) * vertex_color, using_texture);
    float average              = (sampled_texture_color.r + sampled_texture_color.g + sampled_texture_color.b) / 3.0;
    output_color = vec4(average * sin(elapsed_time * 1), average, average * cos(elapsed_time * 1), sampled_texture_color.a * 1.0);
}
