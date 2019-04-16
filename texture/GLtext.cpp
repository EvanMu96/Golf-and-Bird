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
#define WIN_WIDTH   400
#define WIN_HEIGHT  400
#define BMP_Header_Length 54  //图像数据在内存块中的偏移量

GLUquadricObj *pObj1, *pObj2, *pObj3; //quadric objects to store properties of the quadric mesh

//定义两个纹理对象编号
GLuint texGround;
GLuint texWall;
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

// 函数power_of_two用于判断一个整数是不是2的整数次幂
int power_of_two(int n)
{
	if (n <= 0)
		return 0;
	return (n & (n - 1)) == 0;
}

GLuint load_texture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID = 0, texture_ID = 0;

	// 打开文件，如果失败，返回
	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
		return 0;

	// 读取文件中图象的宽度和高度
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// 计算每行像素所占字节数，并根据此数据计算总像素字节数
	{
		GLint line_bytes = width * 3;
		while (line_bytes % 4 != 0)
			++line_bytes;
		total_bytes = line_bytes * height;
	}

	// 根据总像素字节数分配内存
	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	}

	// 读取像素数据
	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// 对就旧版本的兼容，如果图象的宽度和高度不是的整数次方，则需要进行缩放
	// 若图像宽高超过了OpenGL规定的最大值，也缩放
	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width)
			|| !power_of_two(height)
			|| width > max
			|| height > max)
		{
			const GLint new_width = 256;
			const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;

			// 计算每行需要的字节数和总字节数
			new_line_bytes = new_width * 3;
			while (new_line_bytes % 4 != 0)
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;

			// 分配内存
			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if (new_pixels == 0)
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}

			// 进行像素缩放
			gluScaleImage(GL_RGB,
				width, height, GL_UNSIGNED_BYTE, pixels,
				new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

			// 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	// 分配一个新的纹理编号
	glGenTextures(1, &texture_ID);
	if (texture_ID == 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// 绑定新的纹理，载入纹理并设置纹理参数
	// 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
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
	glBindTexture(GL_TEXTURE_2D, lastTextureID);  //恢复之前的纹理绑定
	free(pixels);
	fclose(pFile);
	return texture_ID;
}


void draw_ground(void)
{
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, texWall);
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
	static const GLfloat light_position[] = { 125.0f, 100.0f, 100.0f, 1.0f };
	static const GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	static const GLfloat light_diffuse[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	static const GLfloat light_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

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

void setFog(const GLfloat atmoColor[4])
{
	glEnable(GL_FOG);
	glFogfv(GL_FOG_COLOR, atmoColor);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	//glFogf(GL_FOG_DENSITY, 0.05f);
	glFogf(GL_FOG_START, 1.0f);
	glFogf(GL_FOG_END, 300.0f);
}

void setShadow(void)
{
	m3dGetPlaneEquation(vPlaneEquation, points[0], points[1], points[2]);
	m3dMakePlanarShadowMatrix(shadowMat, vPlaneEquation, vLightPos);
	glMatrixMode(GL_MODELVIEW);
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

void drawscene(void)
{
	const static GLfloat dark_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const static GLfloat white_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const static GLfloat ao_color[] = { 0.7f, 0.9f, 1.0f, 0.8f };
	// set fogcolor
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
	gluLookAt(75, 60, 30, 0, 0, 0, 0, 1, 0);
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

	texWall = load_texture("grassland.bmp");

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
	setShadow();
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

	glutMouseFunc(gsrc_mousebutton);
	glutMotionFunc(gsrc_mousemove);

	glutDisplayFunc(drawscene);   // put everything you wish to draw in drawscene

	glutMainLoop();
}