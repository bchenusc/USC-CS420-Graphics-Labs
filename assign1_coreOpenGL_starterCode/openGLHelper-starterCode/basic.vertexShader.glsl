#version 150

in vec3 position;
in vec4 color;

out vec4 col;

uniform mat4 projectionModelViewMatrix;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  // students need to implement this
  
  gl_Position = projectionModelViewMatrix * vec4(position, 1.0f);
  col = color;
}

