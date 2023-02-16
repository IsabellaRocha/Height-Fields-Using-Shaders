/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: irocha
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>
#include <vector>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y screen coordinates of the current mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;
typedef enum { POINTSMODE, LINESMODE, TRIANGLESMODE, SMOOTHINGMODE } DISPLAY_MODE;
DISPLAY_MODE displayMode = POINTSMODE;

// Transformations of the terrain.
float terrainRotate[3] = { 0.0f, 0.0f, 0.0f }; 
// terrainRotate[0] gives the rotation around x-axis (in degrees)
// terrainRotate[1] gives the rotation around y-axis (in degrees)
// terrainRotate[2] gives the rotation around z-axis (in degrees)
float terrainTranslate[3] = { 0.0f, 0.0f, 0.0f };
float terrainScale[3] = { 1.0f, 1.0f, 1.0f };

// Width and height of the OpenGL window, in pixels.
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

// Stores the image loaded from disk.
ImageIO * heightmapImage;

// VBO and VAO handles.
GLuint vertexPositionAndColorVBO;
GLuint pointVAO, lineVAO, triangleVAO;
GLuint pointVBO, lineVBO, triangleVBO;

vector<float> pointCoordinates, pointColors;
vector<float> lineCoordinates, lineColors;
vector<float> triangelCoordinates, triangleColors;

// CSCI 420 helper classes.
OpenGLMatrix matrix;
BasicPipelineProgram * pipelineProgram;

// Write a screenshot to the specified filename.
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}

void displayFunc()
{
  // This function performs the actual rendering.

  // First, clear the screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set up the camera position, focus point, and the up vector.
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  /*matrix.LookAt(0.0, 0.0, 5.0,
                0.0, 0.0, 0.0, 
                0.0, 1.0, 0.0);
*/
  matrix.LookAt(128, 128, 200,
      0.0, 0.0, 0.0,
      0.0, 1.0, 0.0);
      
  // In here, you can do additional modeling on the object, such as performing translations, rotations and scales.
  // ...

  // Read the current modelview and projection matrices.
  float modelViewMatrix[16];
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(modelViewMatrix);

  float projectionMatrix[16];
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(projectionMatrix);

  // Bind the pipeline program.
  // If an incorrect pipeline program is bound, then the modelview and projection matrices
  // will be sent to the wrong pipeline program, causing shader 
  // malfunction (typically, nothing will be shown on the screen).
  // In this homework, there is only one pipeline program, and it is already bound.
  // So technically speaking, this call is redundant in hw1.
  // However, in more complex programs (such as hw2), there will be more than one
  // pipeline program. And so we will need to bind the pipeline program that we want to use.
  pipelineProgram->Bind(); // This call is redundant in hw1, but it is good to keep for consistency.

  // Upload the modelview and projection matrices to the GPU.
  pipelineProgram->SetModelViewMatrix(modelViewMatrix);
  pipelineProgram->SetProjectionMatrix(projectionMatrix);

  // Execute the rendering.
  //glBindVertexArray(triangleVAO); // Bind the VAO that we want to render.
  //glDrawArrays(GL_TRIANGLES, 0, numVertices); // Render the VAO, by rendering "numVertices", starting from vertex 0.

  glBindVertexArray(pointVAO);
  glDrawArrays(GL_POINTS, 0, pointCoordinates.size() / 3);
  // Swap the double-buffers.
  glutSwapBuffers();
}

void idleFunc()
{
  // Do some stuff... 

  // For example, here, you can save the screenshots to disk (to make the animation).

  // Send the signal that we should call displayFunc.
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  // When the window has been resized, we need to re-set our projection matrix.
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  // You need to be careful about setting the zNear and zFar. 
  // Anything closer than zNear, or further than zFar, will be culled.
  const float zNear = 0.1f;
  const float zFar = 10000.0f;
  const float humanFieldOfView = 60.0f;
  matrix.Perspective(humanFieldOfView, 1.0f * w / h, zNear, zFar);
}

void mouseMotionDragFunc(int x, int y)
{
  // Mouse has moved, and one of the mouse buttons is pressed (dragging).

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the terrain
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        terrainTranslate[0] += mousePosDelta[0] * 0.01f;
        terrainTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        terrainTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the terrain
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        terrainRotate[0] += mousePosDelta[1];
        terrainRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        terrainRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the terrain
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        terrainScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        terrainScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        terrainScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // Mouse has moved.
  // Store the new mouse position.
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // A mouse button has has been pressed or depressed.

  // Keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables.
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // Keep track of whether CTRL and SHIFT keys are pressed.
  switch (glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // If CTRL and SHIFT are not pressed, we are in rotate mode.
    default:
      controlState = ROTATE;
    break;
  }

  // Store the new mouse position.
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // Take a screenshot.
      saveScreenshot("screenshot.jpg");
    break;

    case '1':
        displayMode = POINTSMODE;
        break;
    case '2':
        displayMode = LINESMODE;
        break;
    case '3':
        displayMode = TRIANGLESMODE;
        break;
    case '4':
        displayMode = SMOOTHINGMODE;
        break;
  }
}


void getHeightsFromImage() {
    int imageHeight = heightmapImage->getHeight();
    int imageWidth = heightmapImage->getWidth();
    float heightOfVertex;
    
    for (int i = -imageWidth / 2; i < imageWidth / 2; i++) {
        for (int j = -imageHeight / 2; j < imageHeight / 2; j++) {
            heightOfVertex = heightmapImage->getPixel(i + imageWidth / 2, j + imageHeight / 2, 0) * 0.2;

            //Load x, y, z coordinates of that float into point vector (will be used in VBO)
            pointCoordinates.push_back((float) i);
            pointCoordinates.push_back(heightOfVertex);
            pointCoordinates.push_back((float) -j);

            //Load r, g, b, and alpha, but since it's all white it's just 1, 1, 1, 1
            pointColors.push_back(1.0f);
            pointColors.push_back(1.0f);
            pointColors.push_back(1.0f);
            pointColors.push_back(1.0f);

        }
        
    }
    /*
    for (int idx = 0; idx < pointCoordinates.size(); idx++) {
        if (idx % 3 == 0) {
            cout << endl;
        }
        cout << pointCoordinates[idx] << ", ";
        
    }
    */
}

void initScene(int argc, char *argv[])
{
  // Load the image from a jpeg disk file into main memory.
  heightmapImage = new ImageIO();
  if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
  {
    cout << "Error reading image " << argv[1] << "." << endl;
    exit(EXIT_FAILURE);
  }
  

  // Set the background color.
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black color.
  getHeightsFromImage();
  /*
  for (int idx = 0; idx < pointCoordinates.size(); idx++) {
      if (idx % 3 == 0) {
          cout << endl;
      }
      cout << pointCoordinates[idx] << ", ";

  }
  */
  
  // Enable z-buffering (i.e., hidden surface removal using the z-buffer algorithm).
  glEnable(GL_DEPTH_TEST);

  // Create and bind the pipeline program. This operation must be performed BEFORE we initialize any VAOs.
  pipelineProgram = new BasicPipelineProgram;
  int ret = pipelineProgram->Init(shaderBasePath);
  if (ret != 0) 
  { 
    abort();
  }
  pipelineProgram->Bind();

  // Prepare the triangle position and color data for the VBO. 
  // The code below sets up a single triangle (3 vertices).
  // The triangle will be rendered using GL_TRIANGLES (in displayFunc()).
  /*
  numVertices = 3; // This must be a global variable, so that we know how many vertices to render in glDrawArrays.

  // Vertex positions.
  float * positions = (float*) malloc (numVertices * 3 * sizeof(float)); // 3 floats per vertex, i.e., x,y,z
  positions[0] = 0.0; positions[1] = 0.0; positions[2] = 0.0; // (x,y,z) coordinates of the first vertex
  positions[3] = 0.0; positions[4] = 1.0; positions[5] = 0.0; // (x,y,z) coordinates of the second vertex
  positions[6] = 1.0; positions[7] = 0.0; positions[8] = 0.0; // (x,y,z) coordinates of the third vertex

  // Vertex colors.
  float * colors = (float*) malloc (numVertices * 4 * sizeof(float)); // 4 floats per vertex, i.e., r,g,b,a
  colors[0] = 0.0; colors[1] = 0.0;  colors[2] = 1.0;  colors[3] = 1.0; // (r,g,b,a) channels of the first vertex
  colors[4] = 1.0; colors[5] = 0.0;  colors[6] = 0.0;  colors[7] = 1.0; // (r,g,b,a) channels of the second vertex
  colors[8] = 0.0; colors[9] = 1.0; colors[10] = 0.0; colors[11] = 1.0; // (r,g,b,a) channels of the third vertex
  */

  cout << sizeof(float) * pointCoordinates.size() << endl;
  cout << sizeof(float) * pointColors.size() << endl;
  glGenBuffers(1, &pointVBO);
  glBindBuffer(GL_ARRAY_BUFFER, pointVBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pointCoordinates.size() + sizeof(float) * pointColors.size(), nullptr, GL_STATIC_DRAW);
 
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * pointCoordinates.size(), (float*)pointCoordinates.data());
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * pointCoordinates.size(), sizeof(float) * pointColors.size(), (float*)pointColors.data());

  glGenVertexArrays(1, &pointVAO);
  glBindVertexArray(pointVAO);
  glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
  /*
  // Create the VBOs. There is a single VBO in this example. This operation must be performed BEFORE we initialize any VAOs.
  glGenBuffers(1, &vertexPositionAndColorVBO);
  glBindBuffer(GL_ARRAY_BUFFER, vertexPositionAndColorVBO);
  // First, allocate an empty VBO of the correct size to hold positions and colors.
  const int numBytesInPositions = numVertices * 3 * sizeof(float);
  const int numBytesInColors = numVertices * 4 * sizeof(float);
  glBufferData(GL_ARRAY_BUFFER, numBytesInPositions + numBytesInColors, nullptr, GL_STATIC_DRAW);
  // Next, write the position and color data into the VBO.
  glBufferSubData(GL_ARRAY_BUFFER, 0, numBytesInPositions, positions); // The VBO starts with positions.
  glBufferSubData(GL_ARRAY_BUFFER, numBytesInPositions, numBytesInColors, colors); // The colors are written after the positions.

  // Create the VAOs. There is a single VAO in this example.
  glGenVertexArrays(1, &triangleVAO);
  glBindVertexArray(triangleVAO);
  glBindBuffer(GL_ARRAY_BUFFER, vertexPositionAndColorVBO); // The VBO that we bind here will be used in the glVertexAttribPointer calls below. If we forget to bind the VBO here, the program will malfunction.
  */
  // Set up the relationship between the "position" shader variable and the VAO.
  const GLuint locationOfPosition = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position"); // Obtain a handle to the shader variable "position".
  glEnableVertexAttribArray(locationOfPosition); // Must always enable the vertex attribute. By default, it is disabled.
  const int stride = 0; // Stride is 0, i.e., data is tightly packed in the VBO.
  const GLboolean normalized = GL_FALSE; // Normalization is off.
  glVertexAttribPointer(locationOfPosition, 3, GL_FLOAT, normalized, stride, (const void *)0); // The shader variable "position" receives its data from the currently bound VBO (i.e., vertexPositionAndColorVBO), starting from offset 0 in the VBO. There are 3 float entries per vertex in the VBO (i.e., x,y,z coordinates). 

  // Set up the relationship between the "color" shader variable and the VAO.
  const GLuint locationOfColor = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color"); // Obtain a handle to the shader variable "color".
  glEnableVertexAttribArray(locationOfColor); // Must always enable the vertex attribute. By default, it is disabled.
  glVertexAttribPointer(locationOfColor, 4, GL_FLOAT, normalized, stride, (const void *)(unsigned long) (sizeof(float) * pointCoordinates.size())); // The shader variable "color" receives its data from the currently bound VBO (i.e., vertexPositionAndColorVBO), starting from offset "numBytesInPositions" in the VBO. There are 4 float entries per vertex in the VBO (i.e., r,g,b,a channels). 

  // We don't need this data any more, as we have already uploaded it to the VBO. And so we can destroy it, to avoid a memory leak.
 // free(positions);
 // free(colors);

  // Check for any OpenGL errors.
  std::cout << "GL error: " << glGetError() << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // Perform the initialization.
  initScene(argc, argv);

  // Sink forever into the GLUT loop.
  glutMainLoop();
}


