////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Specification:
//   Display a 31 x 31 quadrilateral mesh for PLANE in perspective projection.
//   Added an axis-angle rotation user-interface, such that the mesh can be rotated by draging the screen.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "glut.h"
#include <Windows.h>

//////////////////////////////////////////////////////////////////
// 
// Include the header file of our rotation user-interface.
// 
#include "gsrc.h"
// 
//////////////////////////////////////////////////////////////////

#define PI 3.141592654
#define GRIDSIZE 31
#define WIN_POSX   50
#define WIN_POSY   100
#define WIN_WIDTH  400
#define WIN_HEIGHT 300

#define IW 1024                    // Image Width
#define IH 499                   // Image Height
unsigned char InputImage[IW][IH][4];
//read raw format image
void ReadRawImage(unsigned char Image[][IH][4])
{
	FILE *fp;
	int  i, j, k;
	unsigned char temp;


	if ((fp = fopen("texture2.raw", "rb")) == NULL)
	{
		printf("Error (ReadImage) : Cannot read the file!!\n");
		exit(1);
	}

	for (i = 0; i<IW; i++)
	{
		for (j = 0; j<IH; j++)
		{
			for (k = 0; k < 3; k++)       // k = 0 is Red  k = 1 is Green K = 2 is Blue
			{
				fscanf(fp, "%c", &temp);
				Image[i][j][k] = (unsigned char)temp;
			}
			Image[i][j][3] = (unsigned char)0;         // alpha = 0.0
		}
	}
	fclose(fp);

}
// read bmp format image
bool readBmp(char *bmpName)
{
	//二进制读方式打开指定的图像文件  
	FILE *fp = fopen(bmpName, "rb");
	if (fp == 0) return 0;
	int bmpWidth, bmpHeight, biBitCount;
	RGBQUAD *pColorTable;
	//跳过位图文件头结构BITMAPFILEHEADER  
	fseek(fp, sizeof(BITMAPFILEHEADER), 0);
	//定义位图信息头结构变量，读取位图信息头进内存，存放在变量head中  
	BITMAPINFOHEADER head;
	fread(&head, sizeof(BITMAPINFOHEADER), 1, fp);
	//获取图像宽、高、每像素所占位数等信息  
	bmpWidth = head.biWidth;
	bmpHeight = head.biHeight;
	biBitCount = head.biBitCount;
	//定义变量，计算图像每行像素所占的字节数（必须是4的倍数）  
	int lineByte = (bmpWidth * biBitCount / 8 + 3) / 4 * 4;
	//灰度图像有颜色表，且颜色表表项为256  
	if (biBitCount == 8) {
		//申请颜色表所需要的空间，读颜色表进内存  
		pColorTable = new RGBQUAD[256];
		fread(pColorTable, sizeof(RGBQUAD), 256, fp);
	}
	
	fread(InputImage, 1, lineByte * bmpHeight, fp);
	//关闭文件  
	fclose(fp);
	return 1;
}


typedef struct point{ // define a structure for 3D point (x, y, z)
	GLfloat x;
	GLfloat y;
	GLfloat z;
} vertex;

vertex mesh [GRIDSIZE][GRIDSIZE];           // define a mesh whose elements are 3D point (x, y, z)


void calculateplane (void)
// calculate the parameters of the plane mesh
{
  for (int i=0;i<GRIDSIZE;i++)
    for (int j=0;j<GRIDSIZE;j++) 
    {
      mesh[i][j].x = 2*float(i)/(GRIDSIZE-1)-1;
      mesh[i][j].y = 2*float(j)/(GRIDSIZE-1)-1;
      mesh[i][j].z = 0;
    }
}

void displayobject (void)
{
  int i, j;
	int K = GRIDSIZE-1;

  //////////////////////////////////////////////////////////////////
  // 
  // Setup perspective projection and the rotation
  // 
  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport ); // viewport is by default the display window
  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 45, double(viewport[2])/viewport[3], 0.1, 10 );
  glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt( 0,0,3, 0,0,0, 0,1,0 );
    glMultMatrixf( gsrc_getmo() );  // get the rotation matrix from the rotation user-interface
  //
  //////////////////////////////////////////////////////////////////

	
	/*  Enable Z buffer method for visibility determination. */
	//  Z buffer code starts
	
    glClear (GL_DEPTH_BUFFER_BIT);
    glEnable (GL_DEPTH_TEST);
	
	// Z buffer code ends */

	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT);  // Clear display window.

	glColor3f(1,0,0);  // Set line segment color to red.
	// load texture information
	ReadRawImage(InputImage);
	// select nearest color components to  texArray
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, IW, IH, 0, GL_RGBA, GL_UNSIGNED_BYTE, InputImage);
	// Advice: draw as few lines as possible. Eliminate all redundant drawing.
	glEnable(GL_TEXTURE_2D);
	

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0,0.0,0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(1.0, 0.0, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glutSwapBuffers();
}

void main (int argc, char** argv)
{

  calculateplane();   // calculate data for the planar mesh

  glutInit (&argc, argv);                                      // Initialize GLUT
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );  // Set display mode.
  glutInitWindowPosition( WIN_POSX, WIN_POSY );                // Set display-window position at (WIN_POSX, WIN_POSY) 
                                                               // where (0, 0) is top left corner of monitor screen
  glutInitWindowSize( WIN_WIDTH, WIN_HEIGHT );		           // Set display-window width and height.
  glutCreateWindow( "OpenGL Program for Quadrilateral Mesh" ); // Create display window.

  //////////////////////////////////////////////////////////////////
  // 
  // Register mouse-click and mouse-move glut callback functions
  // for the rotation user-interface.
  // 
  glutMouseFunc( gsrc_mousebutton );
  glutMotionFunc( gsrc_mousemove );
  //
  //////////////////////////////////////////////////////////////////

  glutDisplayFunc( displayobject );	 // Send planar mesh to display window.
  glutMainLoop();			               // Display everything and wait.
}