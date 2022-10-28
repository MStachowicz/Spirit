#version 330 core

out vec4 FragColor;

uniform float near;
uniform float far;
uniform bool linearDepthView;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    if (linearDepthView)
    {
        float depth = LinearizeDepth(gl_FragCoord.z) / far;
        FragColor = vec4(vec3(depth), 1.0);
    }
    else
    {
        FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    }
}