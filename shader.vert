#version 450 
#extension GL_ARB_separate_shader_objetcs : enable

out gl_PerVertex {
    vec4 gl_Position;
};

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main(){
    gl_Position = positions[gl_VertexIndex], 0.0, 1.0);
}