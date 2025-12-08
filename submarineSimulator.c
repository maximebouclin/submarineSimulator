/************************************************************************************

	File: 			submarineSimulator.c
	
	Description:	Submarine simulation in OpenGl with lighting


	Author:			Maxime Bouclin (PPM Image loader provided by Stephen Brooks)

*************************************************************************************/

#include <stdio.h>
#include <math.h>
#include <GL/freeglut.h>
#include <stdbool.h>
#include <string.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <time.h>



// GLOBAL VARIABLES
bool wireFrameRendering = false;
bool fullScreen = false;
bool fog = true;
int windowWidth = 750;
int windowHeight = 500;
int mouseX = 750/2;
int mouseY = 500/2;
int imageWidth, imageHeight; // texture image size
float t = 0;
float subZ = 0;
float subX = 0;
float subY = 50;
float speed = 0.5; // Speed of the submarine and camera
float camDistance = 100;
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
Object coral1;
Object coral2;
Object coral3;
Object coral4;
Object coral5;
Object coral6;
Object coral7;
Object coral8;
Object coral9;
Object coral10;
Object coral11;
Object coral12;


//Tail of the directional light vector for the sun
GLfloat sunPosition[] = {0.2, 1, 0.2, 0};

//Lighting materials
GLfloat darkYellowAmbient[] = {0.5, 0.5, 0, 0.5};
GLfloat yellowDiffuse[] = {0.9, 0.8, 0, 1};
GLfloat whiteAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat whiteDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat whiteSpecular[] = {1, 1, 1, 1};
GLfloat zeroMaterial[] = {0, 0, 0, 0};
GLfloat redDiffuse[] = {1, 0, 0, 1};
GLfloat greenDiffuse[] = {0, 1, 0, 1};
GLfloat blueDiffuse[] = {0, 0, 1, 1};
GLfloat waterAmbient[] = {0.05f, 0.1f, 0.15f, 1.0f};
GLfloat waterDiffuse[] = {0.0f, 0.4f, 0.7f, 1.0f};
GLfloat waterSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat sandFloorEmission[]   = {0.7f, 0.65f, 0.5f, 1.0f};  // warm sand-ish glow
GLfloat sandWallEmission[]   = {0.4f, 0.4f, 0.4f, 1.0f};  // greyer sand-ish glow
GLfloat noEmission[]     = {0.0f, 0.0f, 0.0f, 1.0f};   // for resetting
GLfloat coralAmbient[]  = {0.05f, 0.15f, 0.05f, 1.0f};  // darker green tint
GLfloat coralDiffuse[]  = {0.10f, 0.45f, 0.15f, 1.0f};  // main green color
GLfloat coralSpecular[] = {0.20f, 0.35f, 0.20f, 1.0f};  // mild shine
GLfloat coralEmission[] = {0.00f, 0.10f, 0.00f, 1.0f};  // soft underwater glow
GLfloat coralShininess  = 20.0f;
GLfloat fishAmbient[]  = {0.02f, 0.05f, 0.10f, 1.0f};   // navy 
GLfloat fishDiffuse[]  = {0.05f, 0.15f, 0.30f, 1.0f};  
GLfloat fishSpecular[] = {0.30f, 0.40f, 0.60f, 1.0f};   
GLfloat fishEmission[] = {0.00f, 0.00f, 0.00f, 1.0f};   
GLfloat fishShininess  = 60.0f;                         

//Shininess coefficients
GLfloat noShininess = 0;
GLfloat waterShininess = 80;
GLfloat highShininess = 100;

//Quadrics
GLUquadricObj* quadPtr;

//Textures
GLuint floorTexID = 0;
GLubyte *imageData;

//Waves
int surfaceHeight = 180;
int tileLen = 3;
int rowNum = 250;
int xWaveNumber = 25;
int xWaveAmplitude = 3;
int zWaveNumber = 10;
int zWaveAmplitude = 2;

//fish params
float swimRadius = 100;
float swimHeight = 100;
float radiusOffset[12];
float heightOffset[12];
float angleOffset[12];
float offsetMaximum = 10;

/************************************************************************

    Function:        drawFish

    Description:     Draw a simple pyramid fish model

*************************************************************************/
void drawFish(){
	GLfloat squareVertex1[] = {0, 0, 0};
	GLfloat squareVertex2[] = {1, 0, 0};
	GLfloat squareVertex3[] = {1, 1, 0};
	GLfloat squareVertex4[] = {0, 1, 0};
	GLfloat pyramidPoint[] = {0.5, 0.5, 3.5};
	glBegin(GL_POLYGON);
		glNormal3f(0,0,1);
		glVertex3fv(squareVertex1);
		glVertex3fv(squareVertex2);
		glVertex3fv(squareVertex3);
		glVertex3fv(squareVertex4);
	glEnd();

	glBegin(GL_TRIANGLES);
		//top
		glNormal3f(0,1,0);
		glVertex3fv(squareVertex3);
		glVertex3fv(squareVertex4);
		glVertex3fv(pyramidPoint);
		//right
		glNormal3f(1,0,0);
		glVertex3fv(squareVertex3);
		glVertex3fv(squareVertex2);
		glVertex3fv(pyramidPoint);
		//left
		glNormal3f(-1,0,0);
		glVertex3fv(squareVertex1);
		glVertex3fv(squareVertex4);
		glVertex3fv(pyramidPoint);
		//bottom
		glNormal3f(0,-1,0);
		glVertex3fv(squareVertex1);
		glVertex3fv(squareVertex2);
		glVertex3fv(pyramidPoint);
	glEnd();

}

/************************************************************************

    Function:        waveHeight

    Description:	Computes a wave height from the position of the
					vertex, the number of waves in the x and z axes,
					the wave amplitudes for those axes, the total
					width of the plane and the time.

*************************************************************************/
float waveHeight(float x, float z, float totalWidth,
                 float xWaveNumber, float xWaveAmplitude,
                 float zWaveNumber, float zWaveAmplitude)
{
    return  sin((xWaveNumber * x * 2 * M_PI / totalWidth) + t) * xWaveAmplitude
          + sin((zWaveNumber * z * 2 * M_PI / totalWidth) + t) * zWaveAmplitude;
}

/************************************************************************

	Function:		loadImage

	Description:	Loads in the PPM image

	Author:			Edward Angel, Edited by Stephen Brooks

*************************************************************************/
void loadImage()
{
	// the ID of the image file
	FILE *fileID;

	// maxValue
	int  maxValue;	
		
	// total number of pixels in the image
	int  totalPixels;

	// temporary character
	char tempChar;

	// counter variable for the current pixel in the image
	int i;

	// array for reading in header information
	char headerLine[100];

	// if the original values are larger than 255
	float RGBScaling;

	// temporary variables for reading in the red, green and blue data of each pixel
	int red, green, blue;

	// open the image file for reading
	fileID = fopen("support_files/sand_ascii.ppm", "r");

	// read in the first header line
	//    - "%[^\n]"  matches a string of all characters not equal to the new line character ('\n')
	//    - so we are just reading everything up to the first line break
	fscanf(fileID,"%[^\n] ", headerLine);

	// make sure that the image begins with 'P3', which signifies a PPM file
	if ((headerLine[0] != 'P') || (headerLine[1] != '3'))
	{
		printf("This is not a PPM file!\n"); 
		exit(0);
	}

	// we have a PPM file
	printf("This is a PPM file\n");

	// read in the first character of the next line
	fscanf(fileID, "%c", &tempChar);

	// while we still have comment lines (which begin with #)
	while(tempChar == '#') 
	{
		// read in the comment
		fscanf(fileID, "%[^\n] ", headerLine);

		// print the comment
		printf("%s\n", headerLine);
		
		// read in the first character of the next line
		fscanf(fileID, "%c",&tempChar);
	}

	// the last one was not a comment character '#', so we need to put it back into the file stream (undo)
	ungetc(tempChar, fileID); 

	// read in the image hieght, width and the maximum value
	fscanf(fileID, "%d %d %d", &imageWidth, &imageHeight, &maxValue);

	// print out the information about the image file
	printf("%d rows  %d columns  max value= %d\n", imageHeight, imageWidth, maxValue);

	// compute the total number of pixels in the image
	totalPixels = imageWidth * imageHeight;

	// allocate enough memory for the image  (3*) because of the RGB data
	imageData = malloc(3 * sizeof(GLuint) * totalPixels);


	// determine the scaling for RGB values
	RGBScaling = 255.0 / maxValue;


	// if the maxValue is 255 then we do not need to scale the 
	//    image data values to be in the range or 0 to 255
	if (maxValue == 255) 
	{
		for(i = 0; i < totalPixels; i++) 
		{
			// read in the current pixel from the file
			fscanf(fileID,"%d %d %d",&red, &green, &blue );

			// store the red, green and blue data of the current pixel in the data array
			imageData[3*totalPixels - 3*i - 3] = red;
			imageData[3*totalPixels - 3*i - 2] = green;
			imageData[3*totalPixels - 3*i - 1] = blue;
		}
	}
	else  // need to scale up the data values
	{
		for(i = 0; i < totalPixels; i++) 
		{
			// read in the current pixel from the file
			fscanf(fileID,"%d %d %d",&red, &green, &blue );

			// store the red, green and blue data of the current pixel in the data array
			imageData[3*totalPixels - 3*i - 3] = red   * RGBScaling;
			imageData[3*totalPixels - 3*i - 2] = green * RGBScaling;
			imageData[3*totalPixels - 3*i - 1] = blue  * RGBScaling;
		}
	}


	// close the image file
	fclose(fileID);
}

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
	if (!file) {
        printf("Error: could not open OBJ file '%s'\n", filename);
        exit(1);
    }

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

	Description:	Checks for key presses for moving the submarine and
					toggling certain settings

*************************************************************************/
void myKeys(unsigned char key, int x, int y) {
	switch (key) {
        case 'w':
			subZ -= speed;
			break;
		case 's': 
			subZ += speed;
			break;
		case 'a':
			subX -= speed;
			break;
		case 'd': 
			subX += speed;
			break;
        case 'u':
			//Toggling between wireframe and filled polygons
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
		case 'b':
			//Toggling between fog and no fog
			fog = !fog;
			if(fog){
				glEnable(GL_FOG);
			}
			else{
				glDisable(GL_FOG);
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

	Description:	Checks for key presses to move the camera and submarine

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

	glutPostRedisplay();
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
	gluPerspective(60.0, (float)windowWidth / windowHeight, 0.1, 2000.0);

}

/************************************************************************

	Function:		myIdle

	Description:	Updates the animation when idle.

*************************************************************************/
void myIdle(){
	t += 0.1;

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
	glDisable(GL_LIGHTING);          // Disable lighting for axes
		glLineWidth(5);
		glBegin(GL_LINES);
			glColor3f(1, 0, 0);
			glVertex3f(-10,0,0);
			glVertex3f(10,0,0);

			glColor3f(0, 1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 10, 0);

			glColor3f(0, 0, 1);
			glVertex3f(0, 0, -10);
			glVertex3f(0, 0, 10);
		glEnd();
	glEnable(GL_LIGHTING);   

	// --------------------  SUBMARINE -------------------- //
    //Material
	glMaterialfv(GL_FRONT, GL_AMBIENT, darkYellowAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecular);
	glMaterialf(GL_FRONT,  GL_SHININESS, highShininess);

	//Drawing
	glLineWidth(2);
    glPushMatrix();
        glTranslatef(subX, subY, subZ);     // move to sub position in world
		glRotatef(-90, 0, 1, 0);				// rotate so that sub is parallel to the z axis
        glScalef(0.2f, 0.2f, 0.2f);         // shrink the model
        drawObject(submarine);
    glPopMatrix();	

	// --------------------  Corals -------------------- //
    //Material
	glMaterialfv(GL_FRONT, GL_AMBIENT, coralAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, coralDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, coralSpecular);
	glMaterialf(GL_FRONT,  GL_SHININESS, coralShininess);

	//Drawing
    glPushMatrix();
		glTranslatef(0, 0, 50);  // move to coral position in world
		glScalef(40, 40, 40); //Scale
        drawObject(coral1); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(-80, 0, 100);  // move to coral position in world
		glScalef(40, 40, 40); //Scale
        drawObject(coral2); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(100, 0, -150);  // move to coral position in world
		glScalef(30, 30, 30); //Scale
        drawObject(coral3); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(-150, 5, 50);  // move to coral position in world
		glScalef(40, 20, 20); //Scale
        drawObject(coral4); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(90, 0, 150);  // move to coral position in world
		glScalef(40, 20, 40); //Scale
        drawObject(coral5); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(-150, 0, -30);  // move to coral position in world
		glScalef(40, 20, 40); //Scale
        drawObject(coral6); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
	    glTranslatef(100, 0, -150);  // move to coral position in world
		glScalef(30, 20, 30); //Scale
        drawObject(coral7); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(190, 0, 60);  // move to coral position in world
		glScalef(60, 60, 60); //Scale
        drawObject(coral8); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(-150, 0, -150);  // move to coral position in world
		glScalef(50, 50, 50); //Scale
        drawObject(coral9); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(-100, 5, 150);  // move to coral position in world
		glScalef(40, 40, 40); //Scale
        drawObject(coral10); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(-40, 0, -160);  // move to coral position in world
		glScalef(40, 40, 40); //Scale
        drawObject(coral11); // Draw
    glPopMatrix();	

	//Drawing
    glPushMatrix();
		glTranslatef(-140, 0, 0);  // move to coral position in world
		glScalef(40, 40, 40); //Scale
        drawObject(coral12); // Draw
    glPopMatrix();	

	// --------------------  OCEAN FLOOR AND WALLS -------------------- //
	
	glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorTexID);

		glMaterialfv(GL_FRONT, GL_AMBIENT,  whiteAmbient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE,  zeroMaterial);
		glMaterialfv(GL_FRONT, GL_SPECULAR, zeroMaterial);
		glMaterialf(GL_FRONT,  GL_SHININESS, 0.0f);
		glMaterialfv(GL_FRONT, GL_EMISSION, sandFloorEmission);

		glPushMatrix();
			glRotatef(90, 1, 0, 0);
			glScalef(30, 30, 1);
			gluDisk(quadPtr, 0, 10, 20, 1);
		glPopMatrix();

		glPushMatrix();
			glTranslatef(0, -5, 0);
			glRotatef(-90, 1, 0, 0);
			glScalef(25, 25, 20);

			glMaterialfv(GL_FRONT, GL_EMISSION, sandWallEmission);
			gluCylinder(quadPtr, 10, 10, 10, 30, 20);
		glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	glMaterialfv(GL_FRONT, GL_EMISSION, noEmission); // reset emission so the submarine doesn’t glow


	// --------------------  WATER SURFACE -------------------- //
	//Material
	glMaterialfv(GL_FRONT, GL_AMBIENT,  waterAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  waterDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, waterSpecular);
	glMaterialf (GL_FRONT, GL_SHININESS, waterShininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);

	int totalWidth = tileLen * rowNum;
	
	glPushMatrix();
		glTranslatef(-(tileLen * rowNum) / 2, surfaceHeight, -(tileLen * rowNum) / 2); // Move the plane to be centered
		//Create a subdivided plane
		for(int currentRow = 0; currentRow < rowNum; currentRow++){
			for(int currentCol = 0; currentCol < rowNum; currentCol++){
				int x = currentCol * tileLen;
				int z = currentRow * tileLen;
				glBegin(GL_POLYGON);
					glNormal3f(0, 1, 0);
					//First vertex of square
					glVertex3f(x, waveHeight(x, z, totalWidth, xWaveNumber, xWaveAmplitude, zWaveNumber, zWaveAmplitude), z);

					//Second vertex
					x += tileLen;
					glVertex3f(x, waveHeight(x, z, totalWidth, xWaveNumber, xWaveAmplitude, zWaveNumber, zWaveAmplitude), z);

					//Third vertex
					z += tileLen;
					glVertex3f(x, waveHeight(x, z, totalWidth, xWaveNumber, xWaveAmplitude, zWaveNumber, zWaveAmplitude), z);

					//Fourth vertex
					x -= tileLen;
					glVertex3f(x, waveHeight(x, z, totalWidth, xWaveNumber, xWaveAmplitude, zWaveNumber, zWaveAmplitude), z);
				glEnd();
			}
		}
	glPopMatrix();

	// --------------------  FISH -------------------- //
	//Material
	glMaterialfv(GL_FRONT, GL_AMBIENT,  fishAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,  fishDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, fishSpecular);
	glMaterialf (GL_FRONT, GL_SHININESS, fishShininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, fishEmission);

	
	
	
	for (int i = 0; i < 12; i++){
		float groupAngleOffset;
		float groupHeightOffset;

		if (i < 3) {
			groupAngleOffset  =   0.0f;
			groupHeightOffset =   -30.0f;
		} else if (i < 6) {
			groupAngleOffset  =  90.0f;
			groupHeightOffset = 60.0f;
		} else if (i < 9) {
			groupAngleOffset  = 180.0f;
			groupHeightOffset = -80.0f;
		} else {
			groupAngleOffset  = 270.0f;
			groupHeightOffset =  30.0f;
		}

		glPushMatrix();
			glRotatef((-20.0f * (t + (angleOffset[i] * 0.2))) + groupAngleOffset, 0, 1, 0);
			glTranslatef(swimRadius + radiusOffset[i],
						swimHeight + heightOffset[i] + groupHeightOffset, 0);
			glScalef(5, 5, 5);
			drawFish();
		glPopMatrix();
	}
	

	glutSwapBuffers();
}

/************************************************************************

	Function:		initializeGL

	Description:	Initializes the OpenGL rendering context for display. 

*************************************************************************/
void initializeGL(){

	// --------------------  LIGHTING -------------------- //
	//Define the sun color and intensity
    GLfloat ambientSun[]	= { 0.0, 0.0, 0.0, 1.0 };  // relying on global ambient
    GLfloat diffuseSun[]	= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specularSun[]	= { 1.0, 1.0, 1.0, 1.0 };

	// set the global ambient light level
    GLfloat globalAmbientLight[] = { 0.4, 0.4, 0.4, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);

	// define the color and intensity for the sun
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientSun);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseSun);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularSun);

    glEnable(GL_LIGHTING); // enable lighting 

    glEnable(GL_LIGHT0); // enable light 0 / sun

	glEnable(GL_DEPTH_TEST); // enable z buffer
	glEnable(GL_NORMALIZE); // make sure the normals are unit vectors

	// --------------------  CAMERA SETUP -------------------- //
	glMatrixMode(GL_PROJECTION); 
	gluPerspective(60.0, (float)windowWidth / windowHeight, 0.1, 2000.0); // set window mode to 3D projection 

	// set up camera position
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0, 5, 20,
			  0, 0, 0,
			  0, 1, 0);

	glClearColor(0, 0, 0, 1);
	glLineWidth(5);

	// --------------------  TEXTURE SETUP -------------------- //
    loadImage(); // load the PPM image

    // generate a texture ID
    glGenTextures(1, &floorTexID);
    glBindTexture(GL_TEXTURE_2D, floorTexID);

	// Use mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // upload the PPM data
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, imageWidth, imageHeight, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    // Set texture environment so it works with lighting
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// --------------------  QUADRATIC SETUP -------------------- //

	//Quadric initialization
	quadPtr = gluNewQuadric();
	gluQuadricDrawStyle(quadPtr, GLU_FILL);
    gluQuadricNormals(quadPtr, GLU_SMOOTH);
    gluQuadricTexture(quadPtr, GL_TRUE);

	// --------------------  FOG SETUP -------------------- //
	glEnable(GL_FOG);
	GLfloat fogColor[] = {0.0f, 0.0f, 0.7f, 1.0f};  // a bluish underwater fog
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogi(GL_FOG_MODE, GL_EXP);
	glFogf(GL_FOG_DENSITY, 0.002f);

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
	// Read in object values
	submarine = readInObject("support_files/submarine.obj");
	coral1 = readInObject("support_files/coral/coral_1.obj");
	coral2 = readInObject("support_files/coral/coral_2.obj");
	coral3 = readInObject("support_files/coral/coral_3.obj");
	coral4 = readInObject("support_files/coral/coral_4.obj");
	coral5 = readInObject("support_files/coral/coral_5.obj");
	coral6 = readInObject("support_files/coral/coral_6.obj");
	coral7 = readInObject("support_files/coral/coral_7.obj");
	coral8 = readInObject("support_files/coral/coral_8.obj");
	coral9 = readInObject("support_files/coral/coral_9.obj");
	coral10 = readInObject("support_files/coral/coral_10.obj");
	coral11 = readInObject("support_files/coral/coral_11.obj");
	coral12 = readInObject("support_files/coral/coral_12.obj");

	// seed the random
	srand(time(NULL));

	// fish random values
	
	for(int i = 0; i < 12; i ++){
		radiusOffset[i] = ((float) rand() / (float) RAND_MAX) * offsetMaximum;
		heightOffset[i] = ((float) rand() / (float) RAND_MAX) * offsetMaximum;
		angleOffset[i] = ((float) rand() / (float) RAND_MAX) * offsetMaximum;
	}

	// go into a perpetual loop
	glutMainLoop(); 
}