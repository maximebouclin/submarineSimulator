/************************************************************************************

	File: 			submarineSimulator.c
	
	Description:	Submarine simulation in OpenGl


	Author:			Maxime Bouclin (Starting code provided by Stephen Brooks)

*************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <GL/freeglut.h>
#include <stdbool.h>
#include <string.h>

// GLOBAL VARIABLES
bool wireFrameRendering = false;
bool fullScreen = false;
int originalWidth = 750;
int originalHeight = 500;
int subVertexCount = 0; //Amount of vertices read in
int subVertexSpace = 100; //Amount of space allocated for vertices
int subNormalCount = 0; //Ammount of normals read in
int subNormalSpace = 100; //Amount of space allocated for normals
int subTriangleCount = 0; //Amount of triangles read in
int subTriangleSpace = 100; //Amount of space allocated for triangles
float (*subVertices)[3];
float (*subNormals)[3];
int (*subTriangles)[3][2];
float t = 0;
float zOffset = 0;
float xOffset = 0;
float yOffset = 0;


/************************************************************************

	Function:		readInTextFile

	Description:	Reads in the file for the enterprise

*************************************************************************/

void readAndParseFile(const char *filename) {
    FILE *file = fopen(filename, "r");

    char line[256];

    while (fgets(line, sizeof(line), file)) { //while there are lines left
        line[strcspn(line, "\n")] = '\0'; //remove newline character at the end of the string

        char *token = strtok(line, " "); //create a token with every space seperated string
        if (!token) continue; //if the token is not empty carry on

        if (strcmp(token, "v") == 0) { //if the label is v for vertex
            for (int i = 0; i < 3; i++) { // go through each token and put the value in the corresponding place in the array
                token = strtok(NULL, " ");
                subVertices[subVertexCount][i] = (GLfloat)atof(token);
            }
            subVertexCount++;

			if (subVertexCount == subVertexSpace -1){
				subVertexSpace += 100;
				subVertices = realloc(subVertices, subVertexSpace * sizeof(float[3]));
			}
        }
		else if (strcmp(token, "vn") == 0) { //if the label is vn for normal
            for (int i = 0; i < 3; i++) { // go through each token and put the value in the corresponding place in the array
                token = strtok(NULL, " ");
                subNormals[subNormalCount][i] = atof(token);
            }
            subNormalCount++;

			if (subNormalCount == subNormalSpace -1){
				subNormalSpace += 100;
				subNormals = realloc(subNormals, subNormalSpace * sizeof(float[3]));
			}
        }
        else if (strcmp(token, "f") == 0) { //if the label is f for face (triangle)

            for (int i = 0; i < 3; i++) { // go through each token and put the value in the corresponding place in the array
                token = strtok(NULL, " ");
				
				int vertexIndex, normalIndex; // for each vertex in a triangle, there is a vertex index and normal index specified
				sscanf(token, "%d//%d", &vertexIndex, &normalIndex);
				
				subTriangles[subTriangleCount][i][0] = vertexIndex; //assign these indices to their place in the triangle array
				subTriangles[subTriangleCount][i][1] = normalIndex;
            }
            subTriangleCount++;

			if (subTriangleCount == subTriangleSpace -1){
				subTriangleSpace += 100;
				subTriangles = realloc(subTriangles, subTriangleSpace * sizeof(int[3][2]));
			}
        }
    }

    fclose(file);

    subVertexCount = subVertexCount; //update global vars
    subTriangleCount = subTriangleCount;
}

/************************************************************************

	Function:		drawSphere

	Description:	Draws a sphere given a size, color, number of stacks and slices for graphics quality

*************************************************************************/
void drawSphere(GLfloat radius, GLint slices, GLint stacks, GLint r, GLint g, GLint b) {
    GLUquadric* quad = gluNewQuadric();          // Create the quadric object
    gluQuadricDrawStyle(quad, GLU_FILL);         // Solid (can also use GLU_LINE)
    gluQuadricNormals(quad, GLU_SMOOTH);   
	glColor3f(r, g, b);     // Smooth shading for lighting
    gluSphere(quad, radius, slices, stacks);     // Draw the actual sphere
    gluDeleteQuadric(quad);                      // Free memory
}


/************************************************************************

	Function:		myIdle

	Description:	Updates the animation when idle.

*************************************************************************/
void myIdle(){

	glutPostRedisplay();
}

/************************************************************************

	Function:		myKeys

	Description:	Checks for key presses for toggling orbit lines and stars

*************************************************************************/
void myKeys(unsigned char key, int x, int y) {
    float speed = 0.5;
	switch (key) {
        case 'w': // orbit lines
			zOffset -= speed;
			break;
		case 's': 
			zOffset += speed;
			break;
		case 'a': // orbit lines
			xOffset -= speed;
			break;
		case 'd': 
			xOffset += speed;
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
				glutReshapeWindow(originalWidth, originalHeight);
				glutPositionWindow(1000, 50);
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

	Function:		myReshape

	Description:	Changes the OpenGL aspect ratio to match a GLUT 
					window resize.

*************************************************************************/
void myReshape(int newWidth, int newHeight)
{
	// update the viewport to still be all of the window
	glViewport (0, 0, newWidth, newHeight);

	// enter GL_PROJECTION mode so that we can change the 2D coordinates 
	glMatrixMode (GL_PROJECTION);

	// clear the projection matrix
	glLoadIdentity ();

	// alter the 2D drawing coordinates so that it matches the shape of the window
	gluPerspective(60.0, (float)newWidth / newHeight, 0.1, 700.0);
}

/************************************************************************

	Function:		initializeGL

	Description:	Initializes the OpenGL rendering context for display. 

*************************************************************************/
void initializeGL(){
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	// set window mode to 3D projection 
	gluPerspective(60.0, (float)originalWidth / originalHeight, 0.1, 700.0);


	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0, 5, 20,
			  0, 0, 0,
			  0, 1, 0);

	glColor3f(1, 1, 1);

	glClearColor(0, 0, 0, 1);
	glLineWidth(2);
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
	gluLookAt(0 + xOffset, 5, 20 + zOffset,
			0 + xOffset, 0, 0 + zOffset,
			0, 1, 0);

	// --------------------  AXIS LINES -------------------- //	
	glBegin(GL_LINES);	
		//x axis line
		glColor3f(1, 0, 0);
		glVertex3f(-1000,0,0);
		glVertex3f(1000,0,0);

		//y axis line
		glColor3f(0, 1, 0);
		glVertex3f(0, -1000, 0);
		glVertex3f(0, 1000, 0);
		
		//z axis line
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, -1000);
		glVertex3f(0, 0, 1000);
	glEnd();
	

	//Small white sphere at the origin
	//drawSphere(1.0f, 32, 32, 1, 1, 1);

	// --------------------  SUBMARINE -------------------- //	
	glColor3f(0.96, 0.94, 0);
	glScalef(0.1, 0.1, 0.1);
	glBegin(GL_TRIANGLES);
		for(int triangleIndx = 0; triangleIndx < subTriangleCount; triangleIndx++){
			for(int vertex = 0; vertex < 3; vertex++){
				// Define the normal vector for the vertex
				int normalIndex = subTriangles[triangleIndx][vertex][1] - 1; // -1 for 1-based indices
				glNormal3fv(subNormals[normalIndex]);
				
				// Define the coordinates for the vertex
				int vertexIndex = subTriangles[triangleIndx][vertex][0] - 1; // -1 for 1-based indices
				glVertex3fv(subVertices[vertexIndex]);

				//printf("drawing a vertex at (%f, %f, %f)\n", subVertices[vertexIndex][0], subVertices[vertexIndex][1], subVertices[vertexIndex][2]);
			}
		}
	glEnd();


    glutSwapBuffers();
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
	glutInitWindowSize(originalWidth, originalHeight);     
	// set window position on screen
	glutInitWindowPosition(1000, 150); 
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
	
	// Initialize arrays for submarine data
	subVertices  = calloc(subVertexSpace,  sizeof *subVertices);
	subNormals   = calloc(subNormalSpace,  sizeof *subNormals);
	subTriangles = calloc(subTriangleSpace, sizeof *subTriangles);

	// Read in submarine values
	readAndParseFile("support_files/submarine.obj");

	// go into a perpetual loop
	glutMainLoop(); 
}