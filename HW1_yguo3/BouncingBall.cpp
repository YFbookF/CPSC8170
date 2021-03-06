/**************************************************
  BouncingBall.cpp
  Student Name: Yang Guo
  Assignment01  CPSC8170  
 **************************************************/
//
// Example program to show how to use Chris Root's OpenGL Camera Class

// Christopher Root, 2006
// Minor Modifications by Donald House, 2009
// Minor Modifications by Yujie Shu, 2012

#include "Camera.h"

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define EPS          0.1


int WIDTH = 640;
int HEIGHT = 480;

int persp_win;

Camera *camera;

bool showGrid = true;

// Declaration of important global variables
static float Box[3] = {16, 16, 16};          // length,width,height of box are all 20
static Vector3d Ball(0, 8, 0);   // initial position of the ball
static float BallRadius = 0.4;

static Vector3d VelocityOrig(10, 0, 0);
static Vector3d Velocity(0, 0, 0);
static Vector3d G(0, -9.8, 0);
static double Mass = 50;
static double Viscosity = 10;
static double CRestitution = 0.6;
static double TimeStep = 0.08;
static int TimerDelay = 100;  // yaogai

// 0,1 2,3,4,5 elements of the array represent front, back, left, right, upper, bottom sides of the box.
static Vector3d PointsPlane[6];
static Vector3d NormsPlane[6];

static Vector3d newBall;

// draws a simple grid
void makeGrid() {
  glColor3f(0.0, 0.0, 0.0);

  glLineWidth(1.0);

  for (float i=-12; i<12; i++) {
    for (float j=-12; j<12; j++) {
      glBegin(GL_LINES);
      glVertex3f(i, 0, j);
      glVertex3f(i, 0, j+1);
      glEnd();
      glBegin(GL_LINES);
      glVertex3f(i, 0, j);
      glVertex3f(i+1, 0, j);
      glEnd();

      if (j == 11){
	glBegin(GL_LINES);
	glVertex3f(i, 0, j+1);
	glVertex3f(i+1, 0, j+1);
	glEnd();
      }
      if (i == 11){
	glBegin(GL_LINES);
	glVertex3f(i+1, 0, j);
	glVertex3f(i+1, 0, j+1);
	glEnd();
      }
    }
  }

  glLineWidth(2.0);
  glBegin(GL_LINES);
  glVertex3f(-12, 0, 0);
  glVertex3f(12, 0, 0);
  glEnd();
  glBegin(GL_LINES);
  glVertex3f(0, 0, -12);
  glVertex3f(0, 0, 12);
  glEnd();
  glLineWidth(1.0);
}

void init() {
  // set up camera
  // parameters are eye point, aim point, up vector
  camera = new Camera(Vector3d(0, 32, 27), Vector3d(0, 0, 0), 
		      Vector3d(0, 1, 0));

  // grey background for window
  glClearColor(0.62, 0.62, 0.62, 0.0);
  glShadeModel(GL_SMOOTH);
  glDepthRange(0.0, 1.0);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  // init value of Points and Norms in each plane, points are chosen to be center of each plane.
  PointsPlane[0] = Vector3d(0, 0, Box[1]/2);
  NormsPlane[0]  = Vector3d(0, 0, -1);

  PointsPlane[1] = Vector3d(0, 0, -Box[1]/2);
  NormsPlane[1]  = Vector3d(0, 0, 1);

  PointsPlane[2] = Vector3d(-Box[0]/2, 0, 0);
  NormsPlane[2]  = Vector3d(1, 0, 0);

  PointsPlane[3] = Vector3d(Box[0]/2, 0, 0);
  NormsPlane[3]  = Vector3d(-1, 0, 0);

  PointsPlane[4] = Vector3d(0, Box[2], 0);
  NormsPlane[4]  = Vector3d(0, -1, 0);

  PointsPlane[5] = Vector3d(0, 0, 0);
  NormsPlane[5]  = Vector3d(0, 1, 0);
  
}


/*
   Load parameter file and reinitialize global parameters
*/
void LoadParameters(char *filename){
  
  FILE *paramfile;
  
  if((paramfile = fopen(filename, "r")) == NULL){
    fprintf(stderr, "error opening parameter file %s\n", filename);
    exit(1);
  }
  
  if(fscanf(paramfile, "%lf %lf %lf %lf %lf %lf %lf",
	    &Mass, &(VelocityOrig.x), &(VelocityOrig.y), &(VelocityOrig.z), &Viscosity, &CRestitution,
	    &TimeStep) != 7){
    fprintf(stderr, "error reading parameter file %s\n", filename);
    exit(1);
  }
  
  fclose(paramfile);
  
  TimerDelay = int(0.5 * TimeStep * 1000);
}

/*
 Initialize the Simulation
 */
void initSimulation(int argc, char* argv[]){

  if(argc != 2){
    fprintf(stderr, "usage: bouncing ball in box param\n");
    exit(1);
  }
  
  LoadParameters(argv[1]);


  Velocity = VelocityOrig;
  Ball = Vector3d(0, Box[2]/2, 0);

}

void drawBox(){
  glBegin(GL_POLYGON);       
  glColor3f(0.0, 0.0, 0.0);
  glVertex3f(-Box[0]/2, 0, Box[1]/2);
  glVertex3f(Box[0]/2, 0, Box[1]/2);
  glVertex3f(Box[0]/2, 0, -Box[1]/2); 
  glVertex3f(-Box[0]/2, 0, -Box[1]/2);
  glEnd();
  
  glBegin(GL_POLYGON);       
  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(-Box[0]/2, 0, -Box[1]/2);
  glVertex3f(Box[0]/2, 0, -Box[1]/2);
  glVertex3f(Box[0]/2, Box[2], -Box[1]/2); 
  glVertex3f(-Box[0]/2, Box[2], -Box[1]/2);
  glEnd();

  glBegin(GL_POLYGON);       
  glColor3f(0.4, 0.4, 0.0);
  glVertex3f(-Box[0]/2, 0, -Box[1]/2);
  glVertex3f(-Box[0]/2, 0, Box[1]/2);
  glVertex3f(-Box[0]/2, Box[2], Box[1]/2); 
  glVertex3f(-Box[0]/2, Box[2], -Box[1]/2);
  glEnd();
  
  glBegin(GL_POLYGON);       
  glColor3f(1.0, 0.0, 0.5);
  glVertex3f(Box[0]/2, 0, -Box[1]/2);
  glVertex3f(Box[0]/2, 0, Box[1]/2);
  glVertex3f(Box[0]/2, Box[2], Box[1]/2); 
  glVertex3f(Box[0]/2, Box[2], -Box[1]/2);
  glEnd();
  
  glBegin(GL_POLYGON);       
  glColor3f(0.7, 0.7, 0.7);
  glVertex3f(Box[0]/2, Box[2], -Box[1]/2);
  glVertex3f(Box[0]/2, Box[2], Box[1]/2);
  glVertex3f(-Box[0]/2, Box[2], Box[1]/2); 
  glVertex3f(-Box[0]/2, Box[2], -Box[1]/2);
  glEnd();
  
  
}

void drawBall(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();


  if (showGrid) 
    makeGrid();
  drawBox();


  glColor3f(1, 0, 0);
  glTranslatef(Ball[0], Ball[1], Ball[2]);
  glutSolidSphere(BallRadius, 50, 50);

  glPopMatrix();
  glFlush;

}

int checkResting(){
  int num = -1;
  for(int i =0; i<6; i++){
    if( Abs((Ball - PointsPlane[i]) * NormsPlane[i] - BallRadius) < EPS &&
	Abs( Velocity * NormsPlane[i]) < EPS ){
      num = i;
    }
  }
  return num+1;
}

bool checkCollision(){
  if(newBall[0] - BallRadius< -Box[0]/2 || newBall[0] + BallRadius> Box[0]/2 ||
     newBall[1] - BallRadius< 0 || newBall[1] + BallRadius > Box[2] ||
     newBall[2] - BallRadius< -Box[1]/2 || newBall[2] + BallRadius> Box[1]/2)
    return true;
  else
    return false;
}




Vector3d Accel(){
  Vector3d accel = G - Viscosity * Velocity / Mass;
  return accel;
  
}


void simulate(int){
  Vector3d acceleration;
  Vector3d newVelocity;
  bool resting;

  acceleration = Accel();

  resting = false;
  int resNum;
  if( resNum = checkResting()){
    resting = true;

    if( acceleration * NormsPlane[resNum-1] > 0){
      resting = false;
    }
    else{
      //std::cout << resNum << endl;
      acceleration = acceleration - (acceleration * NormsPlane[resNum -1]) * NormsPlane[resNum-1];
      Velocity = Velocity - (Velocity * NormsPlane[resNum -1] * NormsPlane[resNum -1]);
      Ball =  Ball - ((Ball - PointsPlane[resNum -1]) * NormsPlane[resNum -1] - BallRadius) *NormsPlane[resNum -1];
    }
  }

  newVelocity = Velocity + TimeStep * acceleration;
  newBall = Ball + TimeStep * Velocity;


  while(checkCollision()){
    int i;
    float rat = 1.0;
    float ratio[6] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    int cNum = -1;	
    for(i =0; i < 6; i++){
      if((Ball - PointsPlane[i]) * NormsPlane[i] - BallRadius < (Ball - newBall) * NormsPlane[i]){
	ratio[i] = ( (Ball - PointsPlane[i]) * NormsPlane[i] - BallRadius) / ( (Ball - newBall) * NormsPlane[i] );
	//std::cout << i << endl;
	//std::cout << ratio[i] << endl;
      }
      if( ratio[i] < rat){
	rat  = ratio[i];
	cNum = i;
      }
    }	
    
    Vector3d velocityC, positionC, v1;
    
    velocityC = Velocity + rat * TimeStep * acceleration; 
    positionC = Ball + rat * TimeStep * Velocity;   	

    v1 = ( velocityC *  NormsPlane[cNum] ) * NormsPlane[cNum] *(-CRestitution )+ (velocityC - ( velocityC *  NormsPlane[cNum] ) * NormsPlane[cNum]);
    Velocity = v1;
    Ball = positionC;
    
    double CollisionTimeStep;
    acceleration = Accel();
    CollisionTimeStep = (1-rat) * TimeStep;
    newVelocity = Velocity + CollisionTimeStep * acceleration; 
    newBall = Ball + CollisionTimeStep * Velocity;

    //std::cout << newBall <<endl;
  }
 
  Ball = newBall;
  Velocity = newVelocity;
  
  //std::cout << "origin" << Ball << endl;

  drawBall();
  glutPostRedisplay();

  glutTimerFunc(TimerDelay, simulate, 1);
  
}




void PerspDisplay() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw the camera created in perspective
  camera->PerspectiveDisplay(WIDTH, HEIGHT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //
  // here is where you would draw your scene!
  //
  glTranslatef(0, -1, 0);
  drawBall();
  
  glutSwapBuffers();
}

void mouseEventHandler(int button, int state, int x, int y) {
  // let the camera handle some specific mouse events (similar to maya)
  camera->HandleMouseEvent(button, state, x, y);
  glutPostRedisplay();
}

void motionEventHandler(int x, int y) {
  // let the camera handle some mouse motions if the camera is to be moved
  camera->HandleMouseMotion(x, y);
  glutPostRedisplay();
}

void keyboardEventHandler(unsigned char key, int x, int y) {
  switch (key) {
  case 'r': case 'R':
    // reset the camera to its initial position
    camera->Reset();
    break;
  case 'f': case 'F':
    camera->SetCenterOfFocus(Vector3d(0, 0, 0));
    break;
  case 'g': case 'G':
    showGrid = !showGrid;
    break;
  case 's': case 'S':
    Velocity = VelocityOrig;
    Ball = Vector3d(0, Box[2]/2, 0);
    //
    glutTimerFunc(TimerDelay, simulate, 1);
    break;
      
  case 'q': case 'Q':	// q or esc - quit
  case 27:		// esc
    exit(0);
  }

  glutPostRedisplay();
}

int main(int argc, char *argv[]) {

  // set up opengl window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(50, 50);
  persp_win = glutCreateWindow("Bouncing Ball In Box");

  // initialize the camera and simulation
  init();
  initSimulation(argc, argv);

  // set up opengl callback functions
  glutDisplayFunc(PerspDisplay);
  glutMouseFunc(mouseEventHandler);
  glutMotionFunc(motionEventHandler);
  glutKeyboardFunc(keyboardEventHandler);

  glutMainLoop();
  return(0);
}

