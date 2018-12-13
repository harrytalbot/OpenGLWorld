
How to operate the program:

All the movement controls using the arrow keys, page up & down and the tour are implemented as specified. Additionally:

- BACKSPACE and ENTER increase/decrease the elavation of the camera.
- R resets all the animation
- T starts and pauses the tour, and E exits the tour. Control is resumed as soon as the tour is terminated.
- L and O run through a list of predefined locations, with P going to the screenshot location. M can be used to return to the camera position, or 
	control can be continued from the pre-defined locations
- V toggles mouse camera control. When V is pressed, the mouse can be used to look around. The left and right arrow keys then have the effect of stepping
	left and right rather than looking left and right. The foward and backaward movement is relative to the camera look direction. Pressing V again 
	returns the mouse and the original controls are applied again. Closing or alt-tabbing out of the application while in this mode will return 
	control of the mouse to the system. I am aware there was a 5 mark penalty for locking the mouse, but the spec also says that the mouse can be used
	to control the camera; seeing as the program gives up the mouse without issues i left this in as an optional feature.
	
(Main) File list:

OpenGLWorld.cpp - main file.
PhysicsEngine.cpp - manages bullet collisons and objects.
Camera.cpp - manages camera movement and position.

include/texture.h - struct for a texture.
include/World.h - defines gravity and COE.

primitives/ShapeData.cpp - struct for a shape object (holds vertex, normal, uv data)
primitives/ShapeGenerator.cpp - used to generate shapes.
primitives/Vertex.h - struct for a vertex.

shaders:


lightsources .vert & .frag - shader for light shading only.
textured .vert & .frag - shader for texturing with an image.
texturedWithLight .vert & .frag - as above but involving light shading (ambient, occlusion, specular).
texturedWithLightTerrain .vert & .frag - as above, but using 3 textures that are mixed and applied depending on the terrain slope.
texturedWater .vert & .frag - as textured, but using a modified vertex shader to create a wave effect on a plane.

Program build is as standard with premake4 and the makefile.

How the program works:

A heightmap image is read in and used to generate a vertex plane for terrain. The terrain is shaded with texturedWithLightTerrain. 
A plane is drawn and textured with texturedWater to simulate lava in the low regions. A bullet plane with bouncing spheres underneath 
simulates balls of lava being thrown out of the lake. A large sphere is textured as space around the outside of the area. Other spheres 
textured as planets are textured and orbit around the main liht source in the centre (the sun). Two spaceship models are loaded, and drawn 
at various locations following regular paths. Small alien-like observatiories are rendered.

