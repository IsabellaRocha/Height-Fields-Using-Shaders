# Assignment 1: Height Fields Using Shaders
### Background of system used
This assignment was written on a Windows device using Visual Studio 2019. To change the command line input, the debugger under properties was used and then to run the Local Windows Debugger was used.

### Basic features
- The image is rendered with the bottom left vertex starting at the origin
- The camera is pointing at the center of the object
- The rendering has 5 modes available
  - Click '1': Points mode
  - Click '2': Lines mode
  - Click '3': Solid mode
  - Click '4': Smoothing mode
  - Click '5': Lines and solid mode (extra)
- Click 'x': Takes a screenshot
- Click and drag: Rotates around the origin (bottom left corner of the object)
- Hold ctrl and click and drag: Translates the object
- Hold shift and click and drag: Scales the image

### Extra features implemented
- The image was reflected such that its orientation matches the original input image instead of rendering its mirror image
- A constant which scales continuously based on the image size was added to the smoothing equation to get rid of extra white triangles
- Solid mode uses GL_TRIANGLE_STRIP instead of GL_TRIANGLES
- A fifth mode in which the wireframe appears in black over the solid object was added
- Handling of colored images was added (Sample images are in the heightmap folder titled color_1.jpg, color_2.jpg, and color_3.jpg)
