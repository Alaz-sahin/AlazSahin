
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartupKey\"" )
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "camera.h"
#include "texture.h"
#include "md2.h"

#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 600
#define D2R 0.0174532
// Perspective Camera Parameters
#define FOVY 70.0f
#define NEAR 1.0f
#define FAR  100.0f

#define  TIMER_PERIOD  16 // Period for the timer.
#define  TIMER_ON     1     // 0:disable timer, 1:enable timer


// Model's states.
#define  STAND  0
#define  RUN    1

// Define object for the model
MD2  models[2];
int current = 0;  // this is the current model
int command = 0;

bool flag = true;

float origin[] = { 0, 0, 0, 1 };


// model position and orientation
typedef struct {
	float x, z, angle;
} position_t;

position_t pos = { 0, -5, 0 };



/* Global Variables for Template File */
bool upKey = false, downKey = false, rightKey = false, leftKey = false;
bool wKey = false, sKey = false, aKey = false, dKey = false, spaceKey = false;
int  winWidth, winHeight; // current Window width and height

Camera cam(0, -3, 9, 0, 0, 0.1); // at the origin, looking at -z, speed: 0.1

Texture ground, rightWall, frontWall, colon, stripe;

MD2 ogre;

float ogreX = 0, ogreZ = 0, ogreDir = 1, ogreAngle = 0;

GLUquadricObj *q; 

void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r*cos(angle), y + r*sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r*cos(angle), y + r*sin(angle));
	}
	glEnd();
}

void print(int x, int y, char *string, void *font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i<len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}


// display text with variables.
void vprint(int x, int y, void *font, char *string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i<len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

void vprint2(int x, int y, float size, char *string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i<len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

void drawFloor() {
	glBindTexture(GL_TEXTURE_2D, ground.id);
	glTranslatef(0, 4, -7);
	glScalef(3, 2, 3);
	for (int i = -4; i<4; i += 1) {
		for (int j = -4; j<4; j += 1) {
			glBegin(GL_QUADS);
			glNormal3f(0, 0, 1);
			glTexCoord2f(0, 0); glVertex3f(i, -5, j);
			glTexCoord2f(1, 0); glVertex3f(i + 1, -5, j);
			glTexCoord2f(1, 1); glVertex3f(i + 1, -5, j + 1);
			glTexCoord2f(0, 1); glVertex3f(i, -5, j + 1);
			glEnd();
			
		}
	}
}

void drawLeftWall() {
	glBindTexture(GL_TEXTURE_2D, frontWall.id);
	glTranslatef(-21, -15, -7);
	glRotatef(90,0,0,1);
	glRotatef(90,0,1,0);	
	glScalef(3, 1, 9);
	for (int i = -4; i<4; i += 1) {		
			glBegin(GL_QUADS);
			glNormal3f(0, 0, 1);
			glTexCoord2f(0, 0); glVertex3f(i, -9, 1);
			glTexCoord2f(1, 0); glVertex3f(i + 1, -9, 1);
			glTexCoord2f(1, 1); glVertex3f(i + 1, -9, 1 + 1);
			glTexCoord2f(0, 1); glVertex3f(i, -9, 1 + 1);
			glEnd();		
	}

}

void drawRightWallUp() {
	glBindTexture(GL_TEXTURE_2D, rightWall.id);
	glTranslatef(3, -9, -7);
	glRotatef(90, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	glScalef(3, 1, 6);
	//glEnable(GL_NORMALIZE);
	for (int i = -4; i<4; i += 1) {
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glTexCoord2f(0, 0); glVertex3f(i, -9, 1);
		glTexCoord2f(1, 0); glVertex3f(i + 1, -9, 1);
		glTexCoord2f(1, 1); glVertex3f(i + 1, -9, 1 + 1);
		glTexCoord2f(0, 1); glVertex3f(i, -9, 1 + 1);
		glEnd();
	}
}

void drawRightWallDown()
{
	glBindTexture(GL_TEXTURE_2D, stripe.id);
	glTranslatef(3, -9, -7);
	glRotatef(90, 0, 0, 1);
	glRotatef(90, 0, 1, 0);
	glScalef(3, 1, 3);
	//glEnable(GL_NORMALIZE);
	for (int i = -4; i<4; i += 1) {
		glBegin(GL_QUADS);
		glNormal3f(0, 0, 1);
		glTexCoord2f(0, 0); glVertex3f(i, -9, 1);
		glTexCoord2f(1, 0); glVertex3f(i + 1, -9, 1);
		glTexCoord2f(1, 1); glVertex3f(i + 1, -9, 1 + 1);
		glTexCoord2f(0, 1); glVertex3f(i, -9, 1 + 1);
		glEnd();
	}
}

void drawFrontColon() {
	glBindTexture(GL_TEXTURE_2D, colon.id);
	glTranslatef(-7.5, -9, -25);
	glRotatef(-90, 1, 0, 0);	
	glScalef(1.5,1.5,1.5);	
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0); glVertex3f(2, -4, 2);
	glTexCoord2f(1, 0); glVertex3f(2 + 6, -4, 2);
	glTexCoord2f(1, 1); glVertex3f(2 + 6, -4, 2 + 6);
	glTexCoord2f(0, 1); glVertex3f(2, -4, 2 + 6);
	glEnd();
}

void drawFrontLeft()
{
	glBindTexture(GL_TEXTURE_2D, frontWall.id);
	glTranslatef(-15, -9, -25);
	glRotatef(-90, 1, 0, 0);
	glScalef(1.5, 1.5, 1.5);
	glBegin(GL_QUADS);
	glNormal3f(0,0,1);
	glTexCoord2f(0, 0); glVertex3f(2, -4, 2);
	glTexCoord2f(1, 0); glVertex3f(2 + 6, -4, 2);
	glTexCoord2f(1, 1); glVertex3f(2 + 6, -4, 2 + 6);
	glTexCoord2f(0, 1); glVertex3f(2, -4, 2 + 6);
	glEnd();
}

void drawfrontRight()
{
	glBindTexture(GL_TEXTURE_2D, frontWall.id);
	glTranslatef(0, -9, -25);
	glRotatef(-90, 1, 0, 0);
	glScalef(1.5, 1.5, 1.5);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0); glVertex3f(2, -4, 2);
	glTexCoord2f(1, 0); glVertex3f(2 + 6, -4, 2);
	glTexCoord2f(1, 1); glVertex3f(2 + 6, -4, 2 + 6);
	glTexCoord2f(0, 1); glVertex3f(2, -4, 2 + 6);
	glEnd();
}
//
// To display onto window using OpenGL commands
//
void display()
{
	static int angle = 0;
	angle++;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	cam.LookAt();
	
	glPushMatrix();
	drawFloor();
	glPopMatrix();

	glPushMatrix();	
	drawLeftWall();
	glPopMatrix();

	glPushMatrix();
	drawRightWallUp();
	glPopMatrix();

	glPushMatrix();
	drawFrontColon();
	glPopMatrix();

	glPushMatrix();
	drawRightWallDown();
	glPopMatrix();


	glPushMatrix();
	drawFrontLeft();
	glPopMatrix();

	glPushMatrix();
	drawfrontRight();
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(ogreX, -4, ogreZ);
	glRotatef(ogreAngle, 0, 1, 0);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	glRotatef(-90.0, 0.0, 0.0, 1.0);
	glScalef(ogreDir, 1, 1);
	glScalef(0.05, 0.05, 0.05);
	ogre.Play(true, true);
	glPopMatrix();
	glEnable(GL_LIGHTING);

	if (flag == true) {
		glPushMatrix();
		glTranslatef(ogreX, -4, ogreZ);
		glRotatef(ogreAngle, 0, 1, 0);
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		glRotatef(-90.0, 0.0, 0.0, 1.0);
		glutWireCylinder(1, 2, 3, 3);
		glPopMatrix();
	}
	
	glutSwapBuffers();


}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeydown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	switch (key) {
	case 'w':
	case 'W': wKey = true; break;
	case 's':
	case 'S': sKey = true; break;
	case 'a':
	case 'A': aKey = true; break;
	case 'd':
	case 'D': dKey = true; break;
	case ' ': spaceKey = true; break;
	}

	if (key == ' ') {
		// play next animation
		command = (command + 1) % 20; // there are 20
		ogre.Do(command); // change animation.

	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyup(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	switch (key) {
	case 'w':
	case 'W': wKey = false; break;
	case 's':
	case 'S': sKey = false; break;
	case 'a':
	case 'A': aKey = false; break;
	case 'd':
	case 'D': dKey = false; break;
	case ' ': spaceKey = false; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}
//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_upKey, GLUT_KEY_downKey, GLUT_KEY_rightKey, GLUT_KEY_rightKey
//
void onSpecialKeydown(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: upKey = true; break;
	case GLUT_KEY_DOWN: downKey = true; break;
	case GLUT_KEY_LEFT: leftKey = true; break;
	case GLUT_KEY_RIGHT: rightKey = true; break;	
	}
	switch (key) {
	case GLUT_KEY_F1: flag = !flag;
		if (!flag) {
			flag = true;
		/*	glEnable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
		}
		else {
			flag = false;
			/*glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);*/
		}
		glutPostRedisplay();
		break;

	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}


//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_upKey, GLUT_KEY_downKey, GLUT_KEY_rightKey, GLUT_KEY_rightKey
//
void onSpecialKeyup(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: upKey = false; break;
	case GLUT_KEY_DOWN: downKey = false; break;
	case GLUT_KEY_LEFT: leftKey = false; break;
	case GLUT_KEY_RIGHT: rightKey = false; break;

	}
	// to refresh the window it calls display() function
	glutPostRedisplay();
}


//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_leftKey_BUTTON , GLUT_rightKey_BUTTON
// states  : GLUT_upKey , GLUT_downKey
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
	// Write your codes here.



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOVY, winWidth * 1.0f / winHeight, NEAR, FAR);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0, 0, 0, 0);
	display(); // refresh window.
}

void onMovedownKey(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function	
	glutPostRedisplay();
}

void onMove(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.
	
	

	if (upKey) {
		ogreX += 0.1 * sin(ogreAngle*D2R);
		ogreZ += 0.1 * cos(ogreAngle * D2R);
		if (ogre.Do() != RUN) {
			ogre.Do(RUN);
			
		}
	}
	else {
		if (ogre.Do() == (RUN)) {
			ogre.Do(STAND);
		}
	}

	if (rightKey) {
		pos.angle -= 3;
		ogreAngle = pos.angle;
	}

	if (leftKey) {
		pos.angle += 3;
		ogreAngle = pos.angle;
	}




	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()

}
#endif

void Init() {
	glEnable(GL_DEPTH_TEST);
	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ogre.Load("model/ogro.md2", "model/ogro.png");

	//Light setup
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat qaAmbientLight[] = { 5, 5, 5, 1};
	GLfloat qaDiffuseLight[] = { 0.2, 0.2, 0.2, 1};
	GLfloat qaSpecularLight[] = { 1.0, 1.0, 1.0, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);
	GLfloat qaLightPosition[] = { ogreX, 0.5, 0, ogreZ };


	// Load Textures
	ground.Load("textures/floor.jpg");
	rightWall.Load("textures/rightWall.jpg");
	frontWall.Load("textures/frontWall.jpg");
	colon.Load("textures/wallColon.jpg");
	stripe.Load("textures/stripe.jpg");

	q = gluNewQuadric();
	//gluQuadricNormals(q, GLU_SMOOTH);
	gluQuadricTexture(q, GL_TRUE);

	glEnable(GL_TEXTURE_2D);
}


void main(int argc, char *argv[])
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Template File");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);
	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeydown);
	glutSpecialFunc(onSpecialKeydown);

	glutKeyboardUpFunc(onKeyup);
	glutSpecialUpFunc(onSpecialKeyup);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutMotionFunc(onMovedownKey);
	glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	Init();

	glutMainLoop();
}