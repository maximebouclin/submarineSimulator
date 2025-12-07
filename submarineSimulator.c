/************************************************************************************

	File: 			submarineSimulator.c
	
	Description:	Submarine simulation in OpenGl


	Author:			Maxime Bouclin

*************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <GL/freeglut.h>
#include <stdbool.h>
#include <string.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// GLOBAL VARIABLES
bool wireFrameRendering = false;
bool fullScreen = false;
int windowWidth = 750;
int windowHeight = 500;
int mouseX = 750/2;
int mouseY = 500/2;
float t = 0;
float subZ = 0;
float subX = 0;
float subY = 0;
float speed = 0.5; // Speed of the submarine and camera
float camDistance = 30;
float camYaw = 0;
float camPitch = 0.3;

//Object struct for drawing blender objects
typedef struct {
	float (*vertices)[3]; //List of the object's vertices (x, y, z for each vertex)
	float (*normals)[3]; //List of the object's normals (x, y, z component for each normal)
	int (*triangles)[3][2]; //List of the object's triangles (3 points per triangle, each point has a vertex and a normal)
	int vertexCount;
	int normalCount;
	int triangleCount;
} Object;

Object submarine;

//Tail of the directional light vector for the sun
GLfloat sunPosition[] = {0, 100, 0, 0};

//Lighting materials
GLfloat darkYellowAmbient[] = {0.5, 0.5, 0, 0.5};
GLfloat yellowDiffuse[] = {0.9, 0.8, 0, 1};
GLfloat whiteSpecular[] = {1, 1, 1, 1};
GLfloat zeroMaterial[] = {0, 0, 0, 0};
GLfloat redDiffuse[] = {1, 0, 0, 1};
GLfloat greenDiffuse[] = {0, 1, 0, 1};
GLfloat blueDiffuse[] = {0, 0, 1, 1};

//Shininess coefficients
GLfloat noShininess = 0;
GLfloat mediumShininess = 50;
GLfloat highShininess = 100;

/************************************************************************

	Function:		drawObject

	Description:	Draws an object given its struct

*************************************************************************/
void drawObject(Object object){
	glBegin(GL_TRIANGLES);
		for(int triangleIndx = 0; triangleIndx < object.triangleCount; triangleIndx++){
			for(int vertex = 0; vertex < 3; vertex++){
				// Define the normal vector for the vertex
				int normalIndex = object.triangles[triangleIndx][vertex][1] - 1; // -1 for 1-based indices
				glNormal3fv(object.normals[normalIndex]);
				
				// Define the coordinates for the vertex
				int vertexIndex = object.triangles[triangleIndx][vertex][0] - 1; // -1 for 1-based indices
				glVertex3fv(object.vertices[vertexIndex]);

				//printf("drawing a vertex at (%f, %f, %f)\n", subVertices[vertexIndex][0], subVertices[vertexIndex][1], subVertices[vertexIndex][2]);
			}
		}
	glEnd();
}


/************************************************************************

	Function:		readInObject

	Description:	Reads in the file for an obj file and return an object
					struct with the information

*************************************************************************/

Object readInObject(const char *filename) {
	Object object = {0}; //Object struct to return
	int objectVertexSpace = 100; //Amount of space allocated for vertices
	int objectNormalSpace = 100; //Amount of space allocated for normals
	int objectTriangleSpace = 100; //Amount of space allocated for triangles

	//Allocate memory for arrays
	object.vertices  = malloc(objectVertexSpace   * sizeof(float[3]));
	object.normals   = malloc(objectNormalSpace   * sizeof(float[3]));
	object.triangles = malloc(objectTriangleSpace * sizeof(int[3][2]));

    FILE *file = fopen(filename, "r"); //Open the file and save the pointer to the file struct

    //Read in file and fill the arrays
	char line[256];
    while (fgets(line, sizeof(line), file)) { //while there are lines left
        line[strcspn(line, "\n")] = '\0'; //remove newline character at the end of the string

        char *token = strtok(line, " "); //create a token with every space seperated string
        if (!token) continue; //if the token is not empty carry on

        if (strcmp(token, "v") == 0) { //if the label is v for vertex
            for (int i = 0; i < 3; i++) { // go through each token and put the value in the corresponding place in the array
                token = strtok(NULL, " ");
                object.vertices[object.vertexCount][i] = (GLfloat)atof(token);
            }
            object.vertexCount++;

			if (object.vertexCount == objectVertexSpace -1){
				objectVertexSpace += 100;
				object.vertices = realloc(object.vertices, objectVertexSpace * sizeof(float[3]));
			}
        }
		else if (strcmp(token, "vn") == 0) { //if the label is vn for normal
            for (int i = 0; i < 3; i++) { // go through each token and put the value in the corresponding place in the array
                token = strtok(NULL, " ");
                object.normals[object.normalCount][i] = atof(token);
            }
            object.normalCount++;

			if (object.normalCount == objectNormalSpace -1){
				objectNormalSpace += 100;
				object.normals = realloc(object.normals, objectNormalSpace * sizeof(float[3]));
			}
        }
        else if (strcmp(token, "f") == 0) { //if the label is f for face (triangle)

            for (int i = 0; i < 3; i++) { // go through each token and put the value in the corresponding place in the array
                token = strtok(NULL, " ");
				
				int vertexIndex, normalIndex; // for each vertex in a triangle, there is a vertex index and normal index specified
				sscanf(token, "%d//%d", &vertexIndex, &normalIndex);
				
				//assign these indices to their place in the triangle array
				object.triangles[object.triangleCount][i][0] = vertexIndex; 
				object.triangles[object.triangleCount][i][1] = normalIndex;
            }
            object.triangleCount++;

			if (object.triangleCount == objectTriangleSpace -1){
				objectTriangleSpace += 100;
				object.triangles = realloc(object.triangles, objectTriangleSpace * sizeof(int[3][2]));
			}
        }
    }
    fclose(file);
	return object;
}

/************************************************************************

	Function:		myMouse

	Description:	Checks for passive mouse motion and rotates the
					camera around the submarine accordingly

*************************************************************************/
void myMouse(int newMouseX, int newMouseY){
	mouseX = newMouseX;
    mouseY = newMouseY;

    float dx = (float)(mouseX - windowWidth  / 2);
    float dy = (float)-1*(mouseY - windowHeight / 2);

    // scale to some sensitivity and convert to radians
    camYaw   = ((2*dx)/windowWidth) * M_PI;
    camPitch = ((2*dy)/windowHeight) * (M_PI/2);

    glutPostRedisplay();
}


/************************************************************************

	Function:		myKeys

	Description:	Checks for key presses for toggling orbit lines and 
					stars

*************************************************************************/
void myKeys(unsigned char key, int x, int y) {
	switch (key) {
        case 'w': // orbit lines
			subZ -= speed;
			break;
		case 's': 
			subZ += speed;
			break;
		case 'a': // orbit lines
			subX -= speed;
			break;
		case 'd': 
			subX += speed;
			break;
        case 'u':
			//Toggling between fireframe and filled polygons
			wireFrameRendering = !wireFrameRendering;
			if(wireFrameRendering){
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
            break;
		case 'f':
			//Toggling between full screen and windowed
			fullScreen = !fullScreen;
			if(fullScreen){
				glutFullScreen();
			} 
			else{
				glutReshapeWindow(750, 500);
				glutPositionWindow(500, 50);
			}
			break;
		case 'q':
			//Quit the application
			exit(0);
			break;
		default:
			break;
    }
	glutPostRedisplay();
}

/************************************************************************

	Function:		mySpecialKeys

	Description:	Checks for key presses to move the camera and enterprise

*************************************************************************/
void mySpecialKeys(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_UP:
			subY += speed;
			break;
		case GLUT_KEY_DOWN:
			subY -= speed;
			break;
		default:
			break;
	}
}

/************************************************************************

	Function:		myReshape

	Description:	Changes the OpenGL aspect ratio to match a GLUT 
					window resize.

*************************************************************************/
void myReshape(int newWidth, int newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;
	
	// update the viewport to still be all of the window
	glViewport (0, 0, windowWidth, windowHeight);

	// enter GL_PROJECTION mode so that we can change the 2D coordinates 
	glMatrixMode (GL_PROJECTION);

	// clear the projection matrix
	glLoadIdentity ();

	// alter the 2D drawing coordinates so that it matches the shape of the window
	gluPerspective(60.0, (float)windowWidth / windowHeight, 0.1, 700.0);
}

/************************************************************************

	Function:		myIdle

	Description:	Updates the animation when idle.

*************************************************************************/
void myIdle(){

	glutPostRedisplay();
}

/************************************************************************

	Function:		myDisplay

	Description:	Display callback, clears frame buffer and depth buffer

*************************************************************************/
void myDisplay(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// --------------------  CAMERA POSITIONING -------------------- //	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Camera position in world space, orbiting around the sub
    float camX = subX + camDistance * cos(camPitch) * sin(camYaw);
    float camY = subY + camDistance * sin(camPitch);
    float camZ = subZ + camDistance * cos(camPitch) * cos(camYaw);

	// Camera looks at the sub
    gluLookAt(camX, camY, camZ,
              subX, subY, subZ,
              0, 1, 0);

	// --------------------  SUN LIGHT POSITIONING -------------------- //	
	glLightfv(GL_LIGHT0, GL_POSITION, sunPosition);

	// --------------------  AXIS LINES -------------------- //	
	glLineWidth(5);
	glBegin(GL_LINES);	
		//x axis line
		glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, redDiffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
		glMaterialf(GL_FRONT,  GL_SHININESS, noShininess);

		glColor3f(1, 0, 0);
		glVertex3f(-1000,0,0);
		glVertex3f(1000,0,0);

		//y axis line
		glMaterialfv(GL_FRONT, GL_DIFFUSE, greenDiffuse);

		glColor3f(0, 1, 0);
		glVertex3f(0, -1000, 0);
		glVertex3f(0, 1000, 0);
		
		//z axis line
		glMaterialfv(GL_FRONT, GL_DIFFUSE, blueDiffuse);

		glColor3f(0, 0, 1);
		glVertex3f(0, 0, -1000);
		glVertex3f(0, 0, 1000);
	glEnd();

	// --------------------  SUBMARINE -------------------- //
    //Matrerial
	glMaterialfv(GL_FRONT, GL_AMBIENT, darkYellowAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecular);
	glMaterialf(GL_FRONT,  GL_SHININESS, highShininess);

	//Drawing
	glLineWidth(2);
    glPushMatrix();
        glTranslatef(subX, subY, subZ);     // move to sub position in world
		glRotatef(-90, 0, 1, 0);				// rotate so that sub is parallel to the z axis
        glScalef(0.1f, 0.1f, 0.1f);         // shrink the model
        drawObject(submarine);
    glPopMatrix();	

    glutSwapBuffers();
}

/************************************************************************

	Function:		initializeGL

	Description:	Initializes the OpenGL rendering context for display. 

*************************************************************************/
void initializeGL(){
	//Define the sun color and intensity
    GLfloat ambientSun[]	= { 0.0, 0.0, 0.0, 1.0 };  // relying on global ambient
    GLfloat diffuseSun[]	= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specularSun[]	= { 1.0, 1.0, 1.0, 1.0 };

	// set the global ambient light level
    GLfloat globalAmbientLight[] = { 0.4, 0.4, 0.4, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);

	// define the color and intensity for the sun
    glLightfv(GL_LIGHT0, GL_AMBIENT,   ambientSun);
    glLightfv(GL_LIGHT0, GL_SPECULAR,  diffuseSun);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,   specularSun);

    glEnable(GL_LIGHTING); // enable lighting 

    glEnable(GL_LIGHT0); // enable light 0 / sun

	glEnable(GL_DEPTH_TEST); // enable z buffer
	glEnable(GL_NORMALIZE); // make sure the normals are unit vectors
	
	// set window mode to 3D projection 
	glMatrixMode(GL_PROJECTION);
	gluPerspective(60.0, (float)windowWidth / windowHeight, 0.1, 700.0);

	// set up camera position
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0, 5, 20,
			  0, 0, 0,
			  0, 1, 0);

	glClearColor(0, 0, 0, 1);
	glLineWidth(5);

}

/************************************************************************

	Function:		main

	Description:	Sets up the openGL rendering context and the windowing
					system, then begins the display loop.

*************************************************************************/
int main(int argc, char** argv){
    // initialize the toolkit
	glutInit(&argc, argv);          
	// set display mode
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); 
	// set window size
	glutInitWindowSize(windowWidth, windowHeight);     
	// set window position on screen
	glutInitWindowPosition(500, 50); 
	// open the screen window
	glutCreateWindow(argv[0]);
	// register redraw function
	glutDisplayFunc(myDisplay);
	//initialize the rendering context
	initializeGL();
	// register reshape function
	glutReshapeFunc(myReshape);
	// register idle function
	glutIdleFunc(myIdle);
	// register keyboard function
	glutKeyboardFunc(myKeys);
	// register special keys function
	glutSpecialFunc(mySpecialKeys);
	// register mouse function
	glutPassiveMotionFunc(myMouse);
	// Read in submarine values
	submarine = readInObject("support_files/submarine.obj");
	// go into a perpetual loop
	glutMainLoop(); 
}