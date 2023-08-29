#version 430 core

void main()
{
     gl_FragDepth = gl_FragCoord.z; // This effectively happens behind the scenes when frag is empty.
}