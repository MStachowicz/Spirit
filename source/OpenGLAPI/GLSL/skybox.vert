#version 330 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4 projection;
uniform mat4 viewNoTranslation;

out vec3 TexCoords;

// Perspective division is performed after the vertex shader has run (dividing gl_Position's xyz coordinates by its w component)
// The z component of the resulting division is equal to that vertex's depth value. We set z component of the output position equal to its w component
// resulting in a z-component that is always equal to 1.0, because when the perspective division is applied its z component translates to w / w = 1.0
// This requires the skybox to be rendered with GL_LEQUAL depth testing because of the above.
void main()
{
    TexCoords = VertexPosition;
    vec4 pos = projection * viewNoTranslation * vec4(VertexPosition, 1.0);
    gl_Position = pos.xyww;
}