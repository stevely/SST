#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 rotateYMatrix;
uniform mat4 rotateXMatrix;
uniform mat4 translateMatrix;
uniform mat4 scaleMatrix;

in vec3 in_Position;
in vec3 in_Normal;

out vec3 pass_Normal;

void main(void)
{
    gl_Position = projectionMatrix * rotateYMatrix * rotateXMatrix
                * scaleMatrix  * translateMatrix
                * vec4(in_Position, 1.0);
    pass_Normal = in_Normal;
}
