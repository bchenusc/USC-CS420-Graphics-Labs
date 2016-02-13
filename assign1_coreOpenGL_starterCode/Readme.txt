Name: Brian Chen
Class: CSCI 420

Keybinds (Possibly extra credit for dynamic loading?):
* Important: Command line still works *
- 'q' to load Heightmap.jpg
- 'w' to load spiral.jpg
- 'e','r', 't', 'y' to load GrandTeton-128.jpg -> GrandTeton-768.jpg
- 'a', 's', 'd', 'f' to load OhioPyle-128.jpg -> OhioPyle-768.jpg
- 'z', 'x', 'c', 'v' to load SantaMonicaMountains-128.jpg -> SantaMonicaMountains-768.jpg

- '-' to Scale the height of the world down.
- '=' to Scale the height of the world up.

- '1' to draw points.
- '2' to draw wires.
- '3' to draw mesh.

Mouse Bindings:
* Important: I removed a lot of original mouse code because I like my controls better *
- LMB Click + Drag to Rotate
- RMB Click + Drag to Translate
- Scroll Wheel to Zoom (possibly extra credit?)

Important Notes:
- Reorganized almost the entire base code to make it easier to read.
- Restructured how VBO's were used in the old code. Vertex, Color, and Index all have their own VBOs.
- I move the camera. I do not move the terrain.
- There are many exposed settings variables like sensitivity that can be tweaked at the top of hw1.cpp.

Extra Credit:
- Used element arrays and glDrawElements (Index buffer for drawing triangles).
- Used GL_TRIANGLE_STRIPS for optimization

Potential Extra Credit Maybe (?):





