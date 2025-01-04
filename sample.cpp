#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>


#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Rachel Hoeferlin
//  Email:			hoeferlr@oregonstate.edu
//  Date:			12/09/24
//  Description:	Final Project for CS450-Blade Runner-esque Animation
//	Topics:			Texturing multiple objects with shader programs, including per-fragment lighting.
//					Various light sources used with different colors, as well as keytime animation
//					on the car object and the camera lookat positions. 

// title of these windows:

const char *WINDOWTITLE = "Final Project -- Rachel Hoeferlin";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 20000;		// 10000 milliseconds = 10 seconds , 20000 ms = 20 seconds


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
#define DEMO_Z_FIGHTING
#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	BoxList;				// object display list
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

GLuint	PillarDL;				 // Pillar Object DL
GLuint	FlyingCarDL;			// Flying Car DL
unsigned char* CarTexture;		// Car Textures
GLuint	CarTex;
unsigned char* SkyTexture;		// Sky Textures
GLuint	SkyTex;

GLuint	SphereDL;				// sphere light source object

GLuint	PyramidDL;				// Pyramid Bldg
GLuint	PyramidTex;
unsigned char* PyramidTexture;
bool	Frozen;

/*struct faces
{
	char* filename;
};
struct faces Faces[] = 
{
	"px.png",
	"nx.png",
	"py.png",
	"ny.png",
	"pz.png",
	"nz.png"

};
*/
// road/grid variables 
unsigned char* GridTexture;
GLuint	GridTex;
GLuint	GridDL;					// grid display list
#define XSIDE	800.f		// length of the x side of the grid
#define X0      (-XSIDE/2.)		// where one side starts
#define NX	1000.f			// how many points in x
#define DX	( XSIDE/(float)NX )	// change in x between the points

#define YGRID	0.f			// y-height of the grid
//changed from 1000 to 600 
#define ZSIDE	800.f			// length of the z side of the grid
#define Z0      (-ZSIDE/2.)		// where one side starts
#define NZ		1000.f		// how many points in z
#define DZ	( ZSIDE/(float)NZ )	// change in z between the points

// Building 0
GLuint	Building0DL;			//Building 1 DL
unsigned char* Building0Texture;		// Building 1 Textures
GLuint	Building0Tex;

// Building 1
GLuint	Building1DL;			//Building 1 DL
unsigned char* Building1Texture;		// Building 1 Textures
GLuint	Building1Tex;

// Building 2
GLuint	Building2DL;			// Building 2 DL
unsigned char* Building2Texture;		// Building 2 Textures
GLuint	Building2Tex;

GLuint	FutureCityDL;
unsigned char* CityTexture;
GLuint	CityTex;

GLuint	RectangleList;		// Rectangle Object
GLuint	signTex;		// Billboard Texture 
unsigned char* signTexture;
GLuint  SignDL;

// Grid object from Kitbash 
//GLuint	Grid2DL;			// Grid2 DL
//unsigned char* Grid2Texture;		// Grid2 Textures
//GLuint	Grid2Tex;

// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);


// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}


float
Ranf( float low, float high )
{
        float r = (float) rand();               // 0 - RAND_MAX
        float t = r  /  (float) RAND_MAX;       // 0. - 1.

        return   low  +  t * ( high - low );
}

// call this if you want to force your program to use
// a different random number sequence every time you run it:
void
TimeOfDaySeed( )
{
	struct tm y2k;
	y2k.tm_hour = 0;    y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 2000; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  now;
	time( &now );
	double seconds = difftime( now, mktime(&y2k) );
	unsigned int seed = (unsigned int)( 1000.*seconds );    // milliseconds
	srand( seed );
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
#include "osucone.cpp"
//#include "osutorus.cpp"
#include "bmptotexture.cpp"
#include "loadobjfile.cpp"
#include "keytime.cpp"
#include "glslprogram.cpp"
//#include "vertexbufferobject.cpp"

GLSLProgram		Car;			// Car shaders
GLSLProgram		Road;				// Road Shaders
GLSLProgram		Bldg;				// Building Shaders

// Keytime variables:
// Car position
Keytimes CarX, CarY, CarZ;
// gluLookat() positions
Keytimes Xeye, Yeye, Zeye;
// scene rotation 
Keytimes SceneRot1;
//Keytimes Color1, Color2, Color3;

// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( NowProjection == ORTHO )
		glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	else
		gluPerspective( 70.f, 1.f,	0.1f,1000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:
	// turn # msec into the cycle ( 0 - MSEC-1 ):
	int msec = glutGet(GLUT_ELAPSED_TIME) % MS_PER_CYCLE;

	// turn that into a time in seconds:
	float nowTime = (float)msec / 1000.;
	// set the eye position, look-at position, and up-vector:
	
	// use keytimes to animate the camera positions
	gluLookAt(Xeye.GetValue(nowTime), Yeye.GetValue(nowTime), Zeye.GetValue(nowTime), 2.f, 5.f, 8.f, 0.f, 1.f, 0.f);
	
	//gluLookAt( -35.f, 3.f, -30.f,     0.f, 5.f, -5.f,     0.f, 1.f, 0.f ); prev/working lookat 2.f, 5.f, 8.f

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0.f, 1.f, 0.f );
	glRotatef( (GLfloat)Xrot, 1.f, 0.f, 0.f );

	// whole scene rotates on the Y axis to give it appearance of camera moving 360d around 
	//glRotatef(SceneRot1.GetValue(nowTime) * sin(.5 * F_PI * Time), 0.f, 1.f, 0.f);
	

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[NowColor][0] );
		glCallList( AxesList );
	}

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable( GL_NORMALIZE );

	//skybox stuff?:
	//glPushMatrix();
	
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, SkyTex);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	//glShadeModel(GL_SMOOTH);
	//SetMaterial(1., 1., 1., 50.);
	//glScalef(100.f, 100.f, 100.f);
	//glCallList(BoxList);
	//glDisable(GL_TEXTURE_2D);
	//glDisable(GL_LIGHTING);
	//glPopMatrix();
	// 
	// Draw the skydome
	/*glPushMatrix();
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, SkyTex);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//glTranslatef(-4000.f, 10.f, -4000.f);
		//SetPointLight(GL_LIGHT0, 1000.f, 1000.f, 1000.f, 1., .53, 0.);
		//glColor3f(1., .53, 0.);
		SetMaterial(1., 1., 1., 4.f);
		glCallList(SphereDL);
		glScalef(50., 50.f, 50.f);
		OsuSphere(50.f, 20.f, 20.f);
		//DrawSphLatLng(50.f, 20.f, 20.f);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
	glPopMatrix();
	*/

	// draw the grid/floor of scene
	glPushMatrix();
	//Road.Use();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glActiveTexture(GL_TEXTURE1);
	// 570/571 to enable texture, 568 + 570 for shaders 
	glBindTexture(GL_TEXTURE_2D, GridTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glShadeModel(GL_SMOOTH);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, GridTex);
	//Road.SetUniformVariable("uRoadTexUnit", 1);
	// draw the grid list:
	glTranslatef(0.f, -4.f, 200.f);
	//SetMaterial(1., .8, 0., 10.f);
	//glScalef(2.f, 2.f, 2.f);
	glRotatef(180.f, 0., 1., 0.);
	SetMaterial(0., .8, 1., 10.f);
	glCallList(GridDL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	//Road.UnUse();
	glPopMatrix();
	
	// floating billboard sign *want it to be the eye picture but not loading??*
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBindTexture(GL_TEXTURE_2D, signTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(-20.0f, 100.0f, 100.0f);
	//SetPointLight(GL_LIGHT0, -20.f, 100.f, 100.f, 0., 1., 1.);
	glScalef(4.f, 4.f, 4.f);
	glCallList(SignDL);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	// Draw the sign light
	glPushMatrix();
	glTranslatef( -20.0f, 100.0f, 100.0f);
	SetPointLight(GL_LIGHT0, -20.0f, 100.0f, 100.0f, 0., .93, 1.);
	//glTranslatef(0., 1000., 100.f);
	glShadeModel(GL_SMOOTH);
	glColor3f(0., .93, 1.);
	glCallList(SphereDL);
	OsuSphere(2.f, 40.f, 40.f);
	//DrawSphLatLng(300.f, 20.f, 20.f);
	glPopMatrix();

	// Draw the OsuSphere object (sun or moon?)
	glPushMatrix();
	glTranslatef(-10., 300., 100.f);
	SetPointLight(GL_LIGHT0, -10., -300., 100., 1., .93, 0.);
		//glTranslatef(0., 1000., 100.f);
	glShadeModel(GL_SMOOTH);
	glColor3f(1., .93, 0.);
	glCallList(SphereDL);
	OsuSphere(15.f, 40.f, 40.f);
		//DrawSphLatLng(300.f, 20.f, 20.f);
	glPopMatrix();

	// Car head lights:
	glPushMatrix();
	// update translate to be fixed to car positions
	//glTranslatef(-25.f, 1.f, -8.f);
	glTranslatef(CarX.GetValue(nowTime), CarY.GetValue(nowTime), CarZ.GetValue(nowTime) + (2.f));
	//SetPointLight(GL_LIGHT0, -25.f, 1.f, -8.f, .95f, .04f, .67f);
	SetSpotLight(GL_LIGHT0, CarX.GetValue(nowTime), CarY.GetValue(nowTime), CarZ.GetValue(nowTime) + (2.f), 0.f, -1.f, 1.f, .95f, .04f, .67f);
	//glTranslatef(0., 1000., 100.f);
	glShadeModel(GL_SMOOTH);
	glColor3f(.95f, .04f, .67f);
	glScalef(.25f, .25f, .25f);
	glCallList(SphereDL);
	OsuSphere(0.5f, 40.f, 40.f);
	//DrawSphLatLng(300.f, 20.f, 20.f);
	glPopMatrix();

	// tail lights
	glPushMatrix();
	//glTranslatef(-25.f, 1.5f, -22.f);
	glTranslatef(CarX.GetValue(nowTime), CarY.GetValue(nowTime)+(.5f), CarZ.GetValue(nowTime) - (2.f));
	SetPointLight(GL_LIGHT0, -25.f, 1.5f, -22.f, .95f, .04f, .67f);
	//SetSpotLight(GL_LIGHT0,CarX.GetValue(nowTime), CarY.GetValue(nowTime)+(.5f), CarZ.GetValue(nowTime) - (2.f), 0.f, -1.f, -1.f, .95f, .04f, .67f);
	//glTranslatef(0., 1000., 100.f);
	glShadeModel(GL_SMOOTH);
	glColor3f(.95f, .04f, .67f);
	glScalef(.25f, .25f, .25f);
	glCallList(SphereDL);
	OsuSphere(0.5f, 40.f, 40.f);
	//DrawSphLatLng(300.f, 20.f, 20.f);
	glPopMatrix();

	// Draw Car Call Shaders
	glPushMatrix();
	Car.Use();
	glActiveTexture(GL_TEXTURE0);		// texture to live on texture unit 0
	glBindTexture(GL_TEXTURE_2D, CarTex);
	Car.SetUniformVariable("uTexUnit", 0);   // tell your shader program to find the texture on texture unit 0
	glTranslatef(CarX.GetValue(nowTime), CarY.GetValue(nowTime), CarZ.GetValue(nowTime));
	//glTranslatef(-25.f, 1.f, -20.f);
	// Rotate car based on keytimes
	glRotatef(SceneRot1.GetValue(nowTime), 0., 1., 0.);
	glRotatef(270.f, 1., 0., 0.);
	
	glScalef(0.2f, 0.2f, 0.2f);
	glCallList(FlyingCarDL);
	Car.UnUse();
	glPopMatrix();
	

	// Draw the OsuSphere object (lighting on top of tower)
	glPushMatrix();
	//glTranslatef(7.5f, 290.f, 207.f);
	SetPointLight(GL_LIGHT0, -115.0f, 62.0f, 285.0f, 1., 0., .5);
	glTranslatef(-115.0f, 62.0f, 285.0f);
	//glShadeModel(GL_SMOOTH);
	glColor3f(1., 0., .5);
	glCallList(SphereDL);
	OsuSphere(2.f, 20.f, 20.f);
	//DrawSphLatLng(8.f, 7.f, 7.f);
	glPopMatrix();

	// building 0 (electric tower obj)
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBindTexture(GL_TEXTURE_2D, CityTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(-140.0f, 0.0f,300.0f);
	glScalef(1.5f, 1.5f, 1.5f);
	glCallList(Building0DL);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();


	// Draw the OsuSphere object (lighting on top of tower)
	glPushMatrix();
	//glTranslatef(7.5f, 290.f, 207.f);
	//SetPointLight(GL_LIGHT0, 100.0f, 60.f, 300.f, 1., 0., .5);
	SetSpotLight(GL_LIGHT0, 110.0f, 62.f, 280.f, 0., 0., - 1., -1., 0., .5);
	glTranslatef(110.0f,62.0f, 280.f);
	//glShadeModel(GL_SMOOTH);
	glColor3f(1., 0., .5);
	glCallList(SphereDL);
	OsuSphere(2.f, 20.f, 20.f);
	//DrawSphLatLng(8.f, 7.f, 7.f);
	glPopMatrix();

	// building 0 (electric tower obj) positioned on either side of pyramid
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBindTexture(GL_TEXTURE_2D, CityTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(90.0f, 0.0f, 300.f);
	glScalef(1.5f, 1.5f, 1.5f);
	glCallList(Building0DL);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	/*// building 1 (*changed to skyscrapers*)
	glPushMatrix();
		Bldg.Use();
		//glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, Building1Tex);
		Bldg.SetUniformVariable("uBldgTexUnit", 4);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTranslatef(-150.0f, 4.0f, 300.0f);
		// maybe scale is off cuz i dont see it or translate (changed from z=75 to z=200)
		glScalef(3.f, 3.f,3.f);
		glCallList(Building1DL);
		glDisable(GL_LIGHTING);
		//glDisable(GL_TEXTURE_2D);
		Bldg.UnUse();
	glPopMatrix();
	*/

	// building 2 (2 buildings)
	/*glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBindTexture(GL_TEXTURE_2D, Building2Tex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(20.f, -2.f, 6.f);
	glRotatef(180.f, 0., 1., 0.);
	glScalef(4.f, 4.f, 4.f);
	glCallList(Building2DL);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix(); */

	// Draw the OsuSphere object (magenta lighting on top of building -x axis)
	glPushMatrix();
		SetPointLight(GL_LIGHT0, -185.f, 121.f, 2.f, 1., 0., .5);
		glTranslatef(-185.f, 121.f, 2.f);
		glShadeModel(GL_SMOOTH);
		// orange (1, .53, 0.)
		glColor3f(1., 0., .5);
		glCallList(SphereDL);
		OsuSphere(2.f, 20.f, 20.f);
		//DrawSphLatLng(8.f, 7.f, 7.f);
	glPopMatrix();

	// City obj
	glPushMatrix();
	Bldg.Use();
	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glBindTexture(GL_TEXTURE_2D, CityTex);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glActiveTexture(GL_TEXTURE2);		//  texture to live on texture unit 2
	glBindTexture(GL_TEXTURE_2D, CityTex);
	Bldg.SetUniformVariable("uBldgTexUnit", 2);
	glTranslatef(30.f, -2.f, -20.f);
	glRotatef(180.f, 0., 1., 0.);
	// cut scale down taking forever to load
	glScalef(1.25f, 1.25f, 1.25f);
	glCallList(FutureCityDL);
	glDisable(GL_LIGHTING);
	//glDisable(GL_TEXTURE_2D);
	Bldg.UnUse();
	glPopMatrix();



	// Draw the OsuSphere object (magenta lighting on top of building +x axis)
	glPushMatrix();
		//glTranslatef(7.5f, 290.f, 207.f);
		SetPointLight(GL_LIGHT0, 22.f, 90.f, 25.f, 1., 0., .5);
		glTranslatef(22.f, 90.f,25.f);
		//glShadeModel(GL_SMOOTH);
		glColor3f(1., 0., .5);
		glCallList(SphereDL);
		OsuSphere(2.f, 20.f, 20.f);
		//DrawSphLatLng(8.f, 7.f, 7.f);
	glPopMatrix();

	// had this scaled to 4 and it encapsulated the scene (so i can do something like that to make skybox?)
	glPushMatrix();
	Bldg.Use();
	//glEnable(GL_TEXTURE_2D);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, PyramidTex);
	Bldg.SetUniformVariable("uBldgTexUnit", 6);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(0.0f, 0.0f,350.0f);
	//glScalef(4.f, 4.f, 4.f);
	glScalef(.02, .02, .02);
	glCallList(PyramidDL);
	//glDisable(GL_LIGHTING);
	///glDisable(GL_TEXTURE_2D);
	Bldg.UnUse();
	glPopMatrix();

	// cyan light on top of building +x axis
	//float radius = 10.0f; // Light radius
	float xlight = 65.f;
	float ylight = 118.0f;  // Fixed height
	float zlight = 70.f;
	glPushMatrix();
	glTranslatef(xlight, ylight, zlight);
	SetPointLight(GL_LIGHT0, 65.f, 118.f, 70.f, 0., 8., 1.);
	//glTranslatef(xlight, ylight, zlight);
	//glPushMatrix();
	//SetPointLight(GL_LIGHT0, -22.f, -10.f, 100.f, .98, .6, .7);
	//glTranslatef(50.f, 2.f, 70.f);
	// orange (1, .53, 0.)
	glShadeModel(GL_SMOOTH);
	glColor3f(0., 8., 1.);
	glCallList(SphereDL);
	OsuSphere(2.f, 20.f, 20.f);
	//DrawSphLatLng(1.f, 7.f, 7.f);
	glPopMatrix();

	
	/*glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBindTexture(GL_TEXTURE_2D, signTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//glTranslatef(20.f, -2.f, 6.f);
	//glRotatef(180.f, 0., 1., 0.);
	//glScalef(4.f, 4.f, 4.f);
	glCallList(Grid2DL);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	*/

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif

	

	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:
	/*
	//glGenTextures(1, &SkyTex);
	int skyWidth, skyHeight;
	char *skyFile = (char*)"sky-orange.bmp";
	unsigned char *SkyTexture = BmpToTexture(skyFile, &skyWidth, &skyHeight);
	if (SkyTexture == NULL)
	{
		fprintf(stderr, "Cannot open texture '%s'\n", "sky-orange.bmp");
	}
	else {
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", "sky-orange.bmp", skyWidth, skyHeight);
		glGenTextures(1, &SkyTex);
		glBindTexture(GL_TEXTURE_2D, SkyTex);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, skyWidth, skyHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, SkyTexture);
	}*/
	/*glPushMatrix();
	for (int i = 0; i < 6; i++)
	{
		int width, height;
		char* skyfile = (char*)Faces[i].filename;
		unsigned char* SkyTexture = BmpToTexture(skyfile, &width, &height);
		if (SkyTexture == nullptr)
		{
			fprintf(stderr, "Cannot open texture '%s'\n", "nightskybox.bmp");
		}
		else {
			fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", "nightskybox.bmp", width, height);
			glGenTextures(1, &SkyTex);
			glBindTexture(GL_TEXTURE_2D, SkyTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, SkyTexture);
		}
	}
	glPopMatrix();
	*/
	

	// grid texture 
	//glGenTextures(1, &GridTex);
	int width, height;
	// changed file from "concrete.bmp" to stone texture one (which is really city tex)
	char* roadfile = (char*)"concrete.bmp";
	//SciFi_Brick"stone-texture.bmp"
	unsigned char *GridTexture = BmpToTexture(roadfile, &width, &height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &GridTex);
	glBindTexture(GL_TEXTURE_2D, GridTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// took out the second 3 in glTexImage2D call and seemed to work 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, GridTexture);
	
	/*Road.Init();
	bool valid = Road.Create("road.vert", "road.frag");
	if (!valid)
	{
		fprintf(stderr, "Yuch!  The Road shader did not compile.\n");
	}
	else
	{
		fprintf(stderr, "Woo-Hoo!  The Road shader compiled.\n");
	}
	Road.SetUniformVariable("uKa", 0.4f); // all 3 should add up to 1.0
	Road.SetUniformVariable("uKd", 0.2f);
	Road.SetUniformVariable("uKs", 0.4f);
	Road.SetUniformVariable("uShininess", 1.f);*/

	// car texture
	//glGenTextures(1, &CarTex);
	int nums, numt;
	char* file = (char*)"DeLorean.bmp";
	unsigned char *CarTexture = BmpToTexture(file, &nums, &numt);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &CarTex);
	glBindTexture(GL_TEXTURE_2D, CarTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// took out the second 3 in glTexImage2D call and seemed to work 
	glTexImage2D(GL_TEXTURE_2D, 0, 3, nums, numt, 0, GL_RGB, GL_UNSIGNED_BYTE, CarTexture);
	
	Car.Init();
	bool carvalid = Car.Create("car.vert", "car.frag");
	if (!carvalid)
	{
		fprintf(stderr, "Yuch!  The Car shader did not compile.\n");
	}
	else
	{
		fprintf(stderr, "Woo-Hoo!  The Car shader compiled.\n");
	}
	Car.SetUniformVariable("uKa", 0.4f); // all 3 should add up to 1.0
	Car.SetUniformVariable("uKd", 0.2f);
	Car.SetUniformVariable("uKs", 0.4f);
	Car.SetUniformVariable("uShininess", 110.f);
	/*
	// Building 0 Textures (3 building structure)
	int building0Width, building0Height;
	char* builidng0file = (char*)"stone-texture.bmp";
	unsigned char *Building0Texture = BmpToTexture(builidng0file, &building0Width, &building0Height);
	if (Building0Texture == nullptr)
	{
		fprintf(stderr, "Cannot open texture '%s'\n", "SciFi_Brick.bmp");
	}
	else {
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", "SciFi_Brick.bmp", building0Width, building0Height);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &Building0Tex);
		glBindTexture(GL_TEXTURE_2D, Building0Tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// took out the second 3 in glTexImage2D call and seemed to work 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, building0Width, building0Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Building0Texture);
	}*/

	/*// Building 1 Textures (skyscrapers)
	int building1Width, building1Height;
	char* building1file = (char*)"buildingtex.bmp";
	unsigned char* Building1Texture = BmpToTexture(building1file, &building1Width, &building1Height);
	if (Building1Texture == nullptr)
	{
		fprintf(stderr, "Cannot open texture '%s'\n", "buildingtex.bmp");
	}
	else {
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", "buildingtex.bmp", building1Width, building1Height);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &Building1Tex);
		glBindTexture(GL_TEXTURE_2D, Building1Tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// took out the second 3 in glTexImage2D call and seemed to work 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, building1Width, building1Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Building1Texture);
	}
	Bldg.Init();
	bool bldg1valid = Bldg.Create("bldg.vert", "bldg.frag");
	if (!bldg1valid)
	{
		fprintf(stderr, "Yuch!  The Bldg shader did not compile.\n");
	}
	else
	{
		fprintf(stderr, "Woo-Hoo!  The Bldg shader compiled.\n");
	}
	Bldg.SetUniformVariable("uKa", 0.3f); // all 3 should add up to 1.0
	Bldg.SetUniformVariable("uKd", 0.2f);
	Bldg.SetUniformVariable("uKs", 0.5f);
	Bldg.SetUniformVariable("uShininess", 75.f);
	*/

	/*
	// Building 2 Textures (2 building structure)
	int building2Width, building2Height;
	char* building2file = (char*)"stone-texture.bmp";
	unsigned char* Building2Texture = BmpToTexture(building2file, &building2Width, &building2Height);
	if (Building2Texture == nullptr)
	{
		fprintf(stderr, "Cannot open texture '%s'\n", "SciFi_Brick.bmp");
	}
	else {
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", "SciFi_Brick.bmp", building2Width, building2Height);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &Building2Tex);
		glBindTexture(GL_TEXTURE_2D, Building2Tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// took out the second 3 in glTexImage2D call and seemed to work 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, building2Width, building2Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Building2Texture);
	}
	*/

	//City Textures
	int CityWidth, CityHeight;
	char* CityFile = (char*)"stone-texture.bmp";
	unsigned char* CityTexture = BmpToTexture(CityFile, &CityWidth, &CityHeight);
	if (CityTexture == nullptr)
	{
		fprintf(stderr, "Cannot open texture '%s'\n", "SciFi_Brick.bmp");
	}
	else {
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", "SciFi_Brick.bmp", CityWidth, CityHeight);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &CityTex);
		glBindTexture(GL_TEXTURE_2D, CityTex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// took out the second 3 in glTexImage2D call and seemed to work 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, CityWidth, CityHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, CityTexture);
	}

	Bldg.Init();
	bool cityvalid = Bldg.Create("bldg.vert", "bldg.frag");
	if (!cityvalid)
	{
		fprintf(stderr, "Yuch!  The Bldg shader did not compile.\n");
	}
	else
	{
		fprintf(stderr, "Woo-Hoo!  The Bldg shader compiled.\n");
	}
	Bldg.SetUniformVariable("uKa", 0.3f); // all 3 should add up to 1.0
	Bldg.SetUniformVariable("uKd", 0.2f);
	Bldg.SetUniformVariable("uKs", 0.5f);
	Bldg.SetUniformVariable("uShininess", 75.f);


	// Pyramid Bldg Textures
	int pyramidWidth, pyramidHeight;
	char* pyramidfile = (char*)"stone-texture.bmp";
	unsigned char* PyramidTexture = BmpToTexture(pyramidfile, &pyramidWidth, &pyramidHeight);
	if (PyramidTexture == nullptr)
	{
		fprintf(stderr, "Cannot open texture '%s'\n", "SciFi_Brick.bmp");
	}
	else {
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", "SciFi_Brick.bmp", pyramidWidth, pyramidHeight);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &PyramidTex);
		glBindTexture(GL_TEXTURE_2D, PyramidTex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// took out the second 3 in glTexImage2D call and seemed to work 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, pyramidWidth, pyramidHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, PyramidTexture);
	}

	Bldg.Init();
	bool pyramidvalid = Bldg.Create("bldg.vert", "bldg.frag");
	if (!pyramidvalid)
	{
		fprintf(stderr, "Yuch!  The Bldg shader did not compile.\n");
	}
	else
	{
		fprintf(stderr, "Woo-Hoo!  The Bldg shader compiled.\n");
	}
	Bldg.SetUniformVariable("uKa", 0.3f); // all 3 should add up to 1.0
	Bldg.SetUniformVariable("uKd", 0.2f);
	Bldg.SetUniformVariable("uKs", 0.5f);
	Bldg.SetUniformVariable("uShininess", 75.f);


	// sign Bldg Textures
	int signWidth, signHeight;
	char* signfile = (char*)"eye-billboard.bmp";
	//"blade-runner-billboard.bmp"
	unsigned char* signTexture = BmpToTexture(signfile, &signWidth, &signHeight);
	if (signTexture == nullptr)
	{
		fprintf(stderr, "Cannot open texture '%s'\n", "eye-billboard.bmp");
	}
	else {
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", "eye-billboard.bmp", signWidth, signHeight);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &signTex);
		glBindTexture(GL_TEXTURE_2D, signTex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// took out the second 3 in glTexImage2D call and seemed to work 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, signWidth, signHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, signTexture);
	}

	
	// animate car's X position
	CarX.Init();
	// car start pos: -25.f, 1.f, -20.f
	// tower +x pos:
	// tower -x pos: -140.0f, 0.0f,300.0f
	// sign pos: -20.0f, 100.0f, 100.0f
	// pyramid pos: 0.0f, 0.0f,350.0f 

	CarX.AddTimeValue(0.0, -25.000);
	CarX.AddTimeValue(1.0, -25.000);
	CarX.AddTimeValue(3.0, -25.500);
	CarX.AddTimeValue(4.0, -27.500);
	CarX.AddTimeValue(5.0, -30.000);
	CarX.AddTimeValue(6.0, -40.000);
	CarX.AddTimeValue(7.0, -75.000);
	//CarX.AddTimeValue(8.0, -100.000);
	CarX.AddTimeValue(8.0, -120.000);
	CarX.AddTimeValue(10.0, -160.000);
	//CarX.AddTimeValue(12.0, -100.500);
	CarX.AddTimeValue(14.0, 120.500);
	CarX.AddTimeValue(16.0, 30.500);
	CarX.AddTimeValue(18.0, 0.500);
	CarX.AddTimeValue(20.0, -25.000);

	// animate car's Y position
	CarY.Init();
	CarY.AddTimeValue(0.0, 1.000);
	CarY.AddTimeValue(1.0, 10.000);
	CarY.AddTimeValue(3.0, 40.000);
	CarY.AddTimeValue(4.0, 60.000);
	CarY.AddTimeValue(5.0, 100.000);
	CarY.AddTimeValue(6.0, 110.000);
	CarY.AddTimeValue(8.0, 100.000);
	CarY.AddTimeValue(10.0, 75.000);
	//CarY.AddTimeValue(12.0, 90.500);
	CarY.AddTimeValue(14.0, 50.500);
	CarY.AddTimeValue(16.0, 30.500);
	CarY.AddTimeValue(18.0, 20.500);
	CarY.AddTimeValue(20.0, 1.000);

	// animate car's Z position
	CarZ.Init();
	CarZ.AddTimeValue(0.0, -20.000);
	CarZ.AddTimeValue(1.0, -20.000);
	CarZ.AddTimeValue(3.0, -20.000);
	CarZ.AddTimeValue(5.0, 90.000);
	CarZ.AddTimeValue(6.0, 160.000);
	CarZ.AddTimeValue(8.0, 220.000);
	CarZ.AddTimeValue(9.0, 300.000);
	CarZ.AddTimeValue(10.0, 360.000);
	CarZ.AddTimeValue(12.0, 300.500);
	CarZ.AddTimeValue(14.0, 200.500);
	CarZ.AddTimeValue(16.0, 100.500);
	CarZ.AddTimeValue(18.0, 20.00);
	CarZ.AddTimeValue(20.0, -20.000);
	
	// animate camera's eye position X, Y, Z variables
	Xeye.Init();
	Xeye.AddTimeValue(0.0, -25.000);
	Xeye.AddTimeValue(1.0, -30.000);
	Xeye.AddTimeValue(2.0,  -30.500);
	Xeye.AddTimeValue(5.0, -20.000);
	Xeye.AddTimeValue(6.0, -70.000);
	Xeye.AddTimeValue(8.0, -110.000);
	Xeye.AddTimeValue(10.0, -170.000);
	Xeye.AddTimeValue(12.0, -110.000);
	Xeye.AddTimeValue(14.0, 130.500);
	Xeye.AddTimeValue(16.0, 40.500);
	Xeye.AddTimeValue(18.0, 20.500);
	Xeye.AddTimeValue(20.0, -25.000);


	

	Yeye.Init();
	Yeye.AddTimeValue(0.0, 1.000);
	Yeye.AddTimeValue(1.0, 10.000);
	Yeye.AddTimeValue(3.0, 40.000);
	Yeye.AddTimeValue(5.0, 100.000);
	Yeye.AddTimeValue(8.0, 130.000);
	Yeye.AddTimeValue(10.0, 100.000);
	Yeye.AddTimeValue(14.0, 70.500);
	Yeye.AddTimeValue(16.0, 40.500);
	Yeye.AddTimeValue(18.0, 20.500);
	Yeye.AddTimeValue(20.0, 1.000);



	Zeye.Init();
	Zeye.AddTimeValue(0.0, -30.000);
	Zeye.AddTimeValue(1.0, -30.000);
	Zeye.AddTimeValue(3.0, -30.000);
	Zeye.AddTimeValue(5.0, 70.000);
	Zeye.AddTimeValue(6.0, 140.000);
	Zeye.AddTimeValue(8.0, 200.000);
	Zeye.AddTimeValue(10.0, 380.000);
	Zeye.AddTimeValue(12.0, 280.000);
	Zeye.AddTimeValue(14.0, 200.500);
	Zeye.AddTimeValue(16.0, 125.500);
	Zeye.AddTimeValue(18.0, 70.00);
	Zeye.AddTimeValue(20.0, -25.000);


	// car rotation
	SceneRot1.Init();
	SceneRot1.AddTimeValue(0.0, 0.f);
	SceneRot1.AddTimeValue(2.0, 0.f);
	SceneRot1.AddTimeValue(5.0, 0.f);
	SceneRot1.AddTimeValue(10.0, 90.f);
	SceneRot1.AddTimeValue(15.0, 180.f);
	SceneRot1.AddTimeValue(20.0, 360.f);
	
	// animated light colors 0., .93, 1.
	/*Color1.Init();
	Color1.AddTimeValue(0.0, 0.f);
	Color1.AddTimeValue(5.0, 0.f);
	Color1.AddTimeValue(10.0, 0.f);
	Color1.AddTimeValue(14.0, 1.f);
	Color1.AddTimeValue(18.0, 1.f);
	Color1.AddTimeValue(20.0, 0.f);

	Color2.Init();
	Color2.AddTimeValue(0.0, 0.93f);
	Color2.AddTimeValue(5.0, 0.93);
	Color2.AddTimeValue(10.0, 0.93);
	Color2.AddTimeValue(14.0, 0.f);
	Color2.AddTimeValue(18.0, 0.f);
	Color2.AddTimeValue(20.0, 0.93);

	Color3.Init();
	Color3.AddTimeValue(0.0, 1.f);
	Color3.AddTimeValue(5.0, 1.f);
	Color3.AddTimeValue(10.0, 1.f);
	Color3.AddTimeValue(14.0, 1.f);
	Color3.AddTimeValue(18.0, 1.f);
	Color3.AddTimeValue(20.0, 1.f);
	*/

	}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;

	// Texture Coords
	float X_min = 0.0;
	float X_max = 1.0;
	float Y_min = 0.0;
	float Y_max = 1.0;

	glutSetWindow( MainWindow );


	// light source sphere 
	SphereDL = glGenLists(1);
	glNewList(SphereDL, GL_COMPILE);
		//glColor3f(1., 1., 1.);
		LoadObjFile("osusphere.cpp");
		OsuSphere(1., 40., 40.);
	glEndList();

	/*PillarDL = glGenLists(1);
	glNewList(PillarDL, GL_COMPILE);
		LoadObjFile((char*)"objPillars.obj");
	glEndList();
	*/

	FlyingCarDL = glGenLists(1);
	glNewList(FlyingCarDL, GL_COMPILE);
		LoadObjFile((char*)"CyberpunkDeLorean.obj");
	glEndList();

	/*Building1DL = glGenLists(1);
	glNewList(Building1DL, GL_COMPILE);
		LoadObjFile((char*)"skyscrapers.obj");
	glEndList();
	*/

	Building0DL = glGenLists(1);
	glNewList(Building0DL, GL_COMPILE);
		LoadObjFile((char*)"electrictower.obj");
	glEndList();

	FutureCityDL = glGenLists(1);
	glNewList(FutureCityDL, GL_COMPILE);
	LoadObjFile((char*)"Futuristic-City.obj");
	glEndList();

	PyramidDL = glGenLists(1);
	glNewList(PyramidDL, GL_COMPILE);
	LoadObjFile((char*)"pyramid-bldg.obj");
	glEndList();

	SignDL = glGenLists(1);
	glNewList(SignDL, GL_COMPILE);
		LoadObjFile((char*)"sign.obj");
	glEndList();

	/*Grid2DL = glGenLists(1);
	glNewList(Grid2DL, GL_COMPILE);
		LoadObjFile((char*)"GRID.obj");
	glEndList();*/

	// create the grid:
	GridDL = glGenLists(1);
	glNewList(GridDL, GL_COMPILE);
	//SetMaterial(0.961, 0.82, 0.557, 30.f);
	glEnable(GL_TEXTURE_2D);
	glNormal3f(0., 1., 0.);
	for (int i = 0; i < NZ; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j < NX; j++)
		{
			//glColor3f(0., 1., 0.);
			float s0 = (X0 + DX * (float)j - X_min) / X_max - X_min;
			float t0 = (Z0 + DZ * (float)(i + 0) - Y_min) / Y_max - Y_min;
			glTexCoord2f(s0, t0);
			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 0));
			float s1 = (X0 + DX * (float)j - X_min) / X_max - X_min;
			float t1 = (Z0 + DZ * (float)(i + 1) - Y_min) / Y_max - Y_min;
			glTexCoord2f(s1, t1);
			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 1));
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glEndList();

	/*
	// Rectangle Object w/ texture vertices (Billboard Sign)
	RectangleList = glGenLists(1);
	glNewList(RectangleList, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);

		glNormal3f(1., 0., 0.);
			float s0 = (dx - X_min) / X_max - X_min;
			float t0 = (dz - Y_min) / Y_max - Y_min;
			glTexCoord2f(s0, t0);
			glVertex3f(dx, -dy*2, dz);
			float s1 = (dx - X_min) / X_max - X_min;
			float t1 = (-dz - Y_min) / Y_max - Y_min;
			glTexCoord2f(s1, t1);
			glVertex3f(dx, -dy*2, -dz);
			float s2 = (dx - X_min) / X_max - X_min;
			float t2 = (-dz - Y_min) / Y_max - Y_min;
			glTexCoord2f(s2, t2);
			glVertex3f(dx, dy*2, -dz);
			float s3 = (dx - X_min) / X_max - X_min;
			float t3 = (dz - Y_min) / Y_max - Y_min;
			glTexCoord2f(s3, t3);
			glVertex3f(dx, dy*2, dz);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glEndList();

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

		glBegin( GL_QUADS );

			glColor3f( 1., 0., 0. );

				glNormal3f( 1., 0., 0. );
					glVertex3f(  dx, -dy,  dz );
					glVertex3f(  dx, -dy, -dz );
					glVertex3f(  dx,  dy, -dz );
					glVertex3f(  dx,  dy,  dz );

				glNormal3f(-1., 0., 0.);
					glVertex3f( -dx, -dy,  dz);
					glVertex3f( -dx,  dy,  dz );
					glVertex3f( -dx,  dy, -dz );
					glVertex3f( -dx, -dy, -dz );

			glColor3f( 0., 1., 0. );

				glNormal3f(0., 1., 0.);
					glVertex3f( -dx,  dy,  dz );
					glVertex3f(  dx,  dy,  dz );
					glVertex3f(  dx,  dy, -dz );
					glVertex3f( -dx,  dy, -dz );

				glNormal3f(0., -1., 0.);
					glVertex3f( -dx, -dy,  dz);
					glVertex3f( -dx, -dy, -dz );
					glVertex3f(  dx, -dy, -dz );
					glVertex3f(  dx, -dy,  dz );

			glColor3f(0., 0., 1.);

				glNormal3f(0., 0., 1.);
					glVertex3f(-dx, -dy, dz);
					glVertex3f( dx, -dy, dz);
					glVertex3f( dx,  dy, dz);
					glVertex3f(-dx,  dy, dz);

				glNormal3f(0., 0., -1.);
					glVertex3f(-dx, -dy, -dz);
					glVertex3f(-dx,  dy, -dz);
					glVertex3f( dx,  dy, -dz);
					glVertex3f( dx, -dy, -dz);

		glEnd( );
		glEndList();
		*/
#ifdef NOTDEF
		glColor3f(1., 1., 1.);
		glBegin(GL_TRIANGLES);
		glVertex3f(-dx, -dy, dz);
		glVertex3f(0., -dy, dz + 0.5f);
		glVertex3f(dx, -dy, dz);
		glEnd();
#endif

	


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'f':
		case 'F':
			Frozen = !Frozen;
			if (Frozen)
				glutIdleFunc(NULL);
			else
				glutIdleFunc(Animate);
			break;
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			NowProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
	Frozen = false;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
