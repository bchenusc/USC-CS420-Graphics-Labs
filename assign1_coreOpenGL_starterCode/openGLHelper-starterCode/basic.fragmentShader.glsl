#version 150

in vec2 tc;
out vec4 c;

uniform sampler2D textureImage;

void main()
{
  // compute the final pixel color
  // students need to implement this
  
  c = texture(textureImage, tc);
}

