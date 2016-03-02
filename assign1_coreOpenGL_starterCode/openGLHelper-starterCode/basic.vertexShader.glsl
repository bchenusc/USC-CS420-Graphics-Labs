#version 150

in vec3 position; // Input vertex position
in vec2 texCoord; // Input texture coordinates

out vec2 tc; // Output texture coordinates

uniform mat4 projectionModelViewMatrix;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  // students need to implement this
  
  gl_Position = projectionModelViewMatrix * vec4(position, 1.0f);
  tc = texCoord;
}

