#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include "math3d.h"
//#include "bmp.h"

#include "glut.h"
#include "gsrc.h"

#define PI			3.141592654  // Prime
#define WIN_POSX    400
#define WIN_POSY    400
#define WIN_WIDTH   640
#define WIN_HEIGHT  480
#define BMP_Header_Length 54  //file information offset

//two reference of texture
GLuint texGrassland;
//animation parameter
int t_prev;
double phi, delta;
M3DMatrix44f shadowMat;
M3DVector4f vPlaneEquation;
M3DVector4d planeEq;
M3DVector3f points[3] = { { -30.0f, 3.0f, -20.0f },
{ -30.0f, 3.0f, 20.0f},
{ 40.0f,3.0f, 20.0f  } };
const M3DVector3f vLightPos = { 125.0f, 100.0f, 100.0f};


GLuint load_image_as_texture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID = 0, texture_ID = 0;

	// read image file, for debugging return 0 for failing
	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
		return 0;

	// read the file information of image
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// calculate total_bytes
	{
		GLint line_bytes = width * 3;
		while (line_bytes % 4 != 0)
			++line_bytes;
		total_bytes = line_bytes * height;
	}

	// allcating memory resource
	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	}

	// read value pixel by pixel
	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}


	// allocate a new texture reference
	glGenTextures(1, &texture_ID);
	if (texture_ID == 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// texture binding
	GLint lastTextureID = last_texture_ID;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, lastTextureID); 
	free(pixels);
	fclose(pFile);
	return texture_ID;
}

// set a plane as ground
void draw_ground(void)
{
	
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texGrassland);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-2000.0f, 1.0f, -2000.0f);
	glTexCoord2f(0.0f, 30.0f); glVertex3f(2000.0f, 1.0f, -2000.0f);
	glTexCoord2f(30.0f, 30.0f); glVertex3f(2000.0f, 1.0f, 2000.0f);
	glTexCoord2f(30.0f, 0.0f); glVertex3f(-2000.0f, 1.0f, 2000.0f);
	glEnd();
	glScalef(4, 0.00002, 4);
	glutSolidCube(100);
	glPopMatrix();
}

void draw_instrument(void)
{
	glPushMatrix();
	glScalef(0.5, 10, 0.5);
	glutSolidCube(30);
	glPopMatrix();
}



void setLight(void)
{
	static const GLfloat light0_position[] = { 125.0f, 100.0f, 100.0f, 1.0f };
	static const GLfloat light0_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	static const GLfloat light0_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	static const GLfloat light0_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

void setMatirial(const GLfloat mat_diffuse[4], GLfloat mat_shininess)
{
	static const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 0.2f };
	static const GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
}

void draw_bird_body(void)
{
	glPushMatrix();
	glScalef(1, 1, 5);
	glutSolidCube(20);
	glPopMatrix();
}

void draw_bird_wing(void)
{
	glPushMatrix();
	glScalef(1, 0.1, 1);
	glutSolidCube(100);
	glPopMatrix();
}

void draw_bird_head()
{
	glPushMatrix();
	glScalef(2, 2, 5);
	glutSolidCube(8);
	glPopMatrix();
	glTranslatef(0, 0, 20);
	glutSolidCone(10, 20, 10, 10);
	glPopMatrix();
}

void draw_bird(void)
{
	glPushMatrix();
	//glColor3f(0.0, 0.0, 0.0);
	draw_bird_body();
	//glColor3f(1.0, 0.3, 0.4);
	glRotatef(delta, 0, 0, 1);
	glTranslatef(60, 0, 0);
	draw_bird_wing();
	glPopMatrix();
	glPushMatrix();
	glScalef(-1, 1, 1);
	glRotatef(30+delta, 0, 0, 1);
	glTranslatef(60, 0, 0);
	draw_bird_wing();
	glPopMatrix();
	glPushMatrix();
	glRotatef(-30, 1, 0, 0);
	glTranslatef(0, -10, 55);
	//glColor3f(0, 0, 1.0);
	draw_bird_head();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0, 0, -50);
	glRotated(-90, 0, 0, 1);
	glutSolidCone(10, 20, 10, 10);
	glPopMatrix();
}

// set fog effect
void setFog(const GLfloat atmoColor[4])
{
	glEnable(GL_FOG);
	glFogfv(GL_FOG_COLOR, atmoColor);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	//glFogf(GL_FOG_DENSITY, 0.05f);
	glFogf(GL_FOG_START, 1.0f);
	glFogf(GL_FOG_END, 300.0f);
}

//set shadoweffect by library
void setShadow(void)
{
	m3dGetPlaneEquation(vPlaneEquation, points[0], points[1], points[2]);
	m3dMakePlanarShadowMatrix(shadowMat, vPlaneEquation, vLightPos);
	glMatrixMode(GL_MODELVIEW);
	//just for test, please comment this before build
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glMultMatrixf((GLfloat*)shadowMat);
	glTranslatef(-50 + phi, 13, 0);
	glColor3f(0.0, 0.0, 0.0);
	glutSolidSphere(10, 10, 10);
	glEnable(GL_LIGHTING);
	glPopMatrix();
	
}

//set shadow effect by code from slide
void setShadowSlide(void)
{
	GLfloat ProjM[16];
	int i;
	for (i = 0; i < 16; i++)
	{
		ProjM[i] = 0;

	}
	ProjM[0] = ProjM[5] = ProjM[10] = 1;
	ProjM[7] = -1.0 / 100;
	//set parameters
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	//just for test, please comment this before build
	//glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	glTranslatef(0, 3, 0);
	glTranslated(125, 100, 100);
	glMultMatrixf(ProjM);
	glTranslated(-125, -100, -100);
	glTranslatef(-50 + phi, 13+2, 0);
	glColor3f(0.0, 0.0, 0.0);
	glutSolidSphere(10, 10, 10);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

//animation function to modify parameters
void animate(void)
// this animation function will translate the ball with 90 distance
{
	double	t;
	double distance = 90.0;                  // ditance
	double swing_angle = 60;
	double swing_time = 10000.0;				 // 5000 ms

	t = glutGet(GLUT_ELAPSED_TIME) - t_prev;            // return elapsed time in ms since the last call  

	if (t < swing_time)
	{
		// identical motion
		//phi = swing_angle * t / swing_time;
		// accelaration
		phi = distance * (1 - pow(cos(PI * t / (2 * swing_time)), 10.0));
		delta = swing_angle * pow(cos(PI *15* t / (2 * swing_time)), 3.0);
	}
	else
		phi = distance;                     // stop at swing_angle

	glutPostRedisplay();
}

//main dispaly function
void drawscene(void)
{
	const static GLfloat dark_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const static GLfloat white_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const static GLfloat ao_color[] = { 0.7f, 0.9f, 1.0f, 0.8f };
	// set fogcolor as white
	GLfloat atmoColor[4] = { 1.0, 1.0, 1.0, 1.0 };
	// Setup perspective projection and the rotation
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// viewport is by default the display window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, double(viewport[2]) / viewport[3], 1, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(80, 65, 40, 0, 0, 0, 0, 1, 0);
	glMultMatrixf(gsrc_getmo());

	// Clear The Screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// The Blend Factor
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// The Texture Factor
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);   // normalize normals
	glEnable(GL_TEXTURE_2D);			// texture

	texGrassland = load_image_as_texture("grassland.bmp");

	// SetLight
	setLight();
	//setFog(atmoColor);
	glDepthMask(GL_FALSE);// Z buffer code ends */

	glClearColor(0.764, 0.854, 1.0, 1.0);	// Set display-window color to white.
	glClear(GL_COLOR_BUFFER_BIT);		// Clear display window.
	//Draw the base


	glPushMatrix();
	setMatirial(dark_color, 30.0);
	glDepthMask(GL_TRUE);
	draw_ground();
	glDisable(GL_TEXTURE_2D);
	//draw_instrument();
	setMatirial(white_color, 30);
	glTranslatef(-50+phi, 13, 0);
	glutSolidSphere(10, 10, 10);
	glPopMatrix();
	glPushMatrix();
	glScalef(0.2, 0.2, 0.2);
	glTranslatef(70, 90, 90);
	setMatirial(ao_color, 40);
	draw_bird();
	glPopMatrix();
	//setMatirial(dark_color, 40);
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor3f(0, 0, 0);
	setShadowSlide();
	glPopMatrix();
	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);			                      // Initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Set display mode
	glutInitWindowPosition(WIN_POSX, WIN_POSY);         // Set display-window position at (WIN_POSX, WIN_POSY) 
														  // where (0, 0) is top left corner of monitor screen
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);		  // Set display-window width and height.
	glutCreateWindow("Bird and Golf");					  // Create display window.

	// set animation
	glutIdleFunc(animate);

	//mouse function
	glutMouseFunc(gsrc_mousebutton);
	glutMotionFunc(gsrc_mousemove);

	glutDisplayFunc(drawscene);   // put everything you wish to draw in drawscene

	glutMainLoop();
}