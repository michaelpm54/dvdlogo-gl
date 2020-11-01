#version 450 core

out vec4 colour;

uniform sampler2D tex;

void main()
{
    colour = texture(tex, gl_PointCoord);
}
