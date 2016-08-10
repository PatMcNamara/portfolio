/* Patrick McNamara */
/* cs4410 HW 4 */
/* 01/12/2015 */
using namespace std;
#define _USE_MATH_DEFINES
#include <math.h>
#include <list>
#include <time.h> // Used to seed RNG

#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>

#include "RGBpixmap.h"

struct GLfloatPoint {
	GLfloat x, y, z;
};

struct vector {
	float x, y, z;
};

struct sphere {
	GLUquadric* quad;
	struct GLfloatPoint center;
	struct vector v;
	float radius;
	float xroll;
	float dxroll;
};

const GLfloatPoint EYE = { 0, 15, -30 };

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const float VIEW_ANGLE = 30.0;
const float NEAR_PLANE = 20;
const float FAR_PLANE = 1000.0;

list <struct sphere> spheres;//TODO you need to delete each sphere as it gets removed
const float SPHERE_RADIUS = 5;
const int NUM_SPHERES = 8; // Number of spheres coming at player.
RGBpixmap balltexture;

// How much z varies between each call to idle func.
const float BALL_SPEED = .1; // The higher this is, the faster the balls move.
const float BALL_SPIN_SPEED = .2;

GLUquadricObj *cannon;
struct GLfloatPoint cannonLocation;
int xroll, yroll;
int cannonMovingBack, cannonMovingForward;
const float CANNON_MOVEMENT_DISTANCE = 6;
const float CANNON_MOVE_BACK_SPEED = BALL_SPEED;
const float CANNON_MOVE_FORWARD_SPEED = .5;
const float CANNON_BARREL_LENGTH = 10;
const float CANNON_ROTATE_SPEED = 1;
const float MAX_CANNON_ROTATION = 20;

int score;
const int SCORE_INCREMENTS = 100;
bool gameOver = false;

const int SLICES = 40;
const int STACKS = 40;

void drawText(int x, int y, char* str) {
	// Disable all lighting and textures
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	
	// Save state and set projection to ortho.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glColor3f(1.0, 0, 0);
	glRasterPos4i(x, y, 1, 1);
	for (char *c = str; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}

	// Restore state
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	// Reenable all lighting and texturing.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}

/* Calculates a vector from the center of s to end and stores it in s->v */
void calculateUnitVector(struct sphere *s, GLfloatPoint end) {
	float dx = end.x - s->center.x,
	      dy = end.y - s->center.y,
	      dz = end.z - s->center.z;
	float vlen = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
	s->v.x = dx / vlen;
	s->v.y = dy / vlen;
	s->v.z = dz / vlen;
}

// Gets an x,y on the z plane that can be seen by the user.
void getRandPointInViewVolume(GLfloatPoint *center) {
	float maxLocation = atan((VIEW_ANGLE / 2) * M_PI / 180) * center->z * 2;
	float maxLocationX = maxLocation * (float)SCREEN_WIDTH / SCREEN_HEIGHT;
	float maxLocationY = maxLocation * (float)SCREEN_HEIGHT / SCREEN_WIDTH;
	center->x = (rand() % (int)maxLocationX) - (maxLocationX / 2);
	center->y = (rand() % (int)maxLocationY) - (maxLocationY / 2);
}

void initTargets() {
	// Create random spheres
	srand(time(NULL));
	for (int i = 0; i < NUM_SPHERES; i++) {
		sphere s;
		s.radius = SPHERE_RADIUS;
		s.center.z = FAR_PLANE - s.radius;// TODO start a sphere at a random z at first, but make sure it isn't too close to the near plane.
		getRandPointInViewVolume(&s.center);
		calculateUnitVector(&s, EYE);

		// The sphere is actually drawn here
		s.quad = gluNewQuadric();
		gluQuadricDrawStyle(s.quad, GLU_FILL);
		gluQuadricTexture(s.quad, GLU_TRUE);
		gluSphere(s.quad, s.radius, SLICES, STACKS);

		spheres.push_front({s.quad, { s.center.x, s.center.y, s.center.z },{ s.v.x, s.v.y, s.v.z }, s.radius, 0, BALL_SPIN_SPEED });//TODO this is very unC++
	}
}

// Init the openGL system.
void init() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(VIEW_ANGLE, (float) SCREEN_WIDTH / SCREEN_HEIGHT, NEAR_PLANE, FAR_PLANE);

	// Set up camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(EYE.x, EYE.y, EYE.z, EYE.x, EYE.y, FAR_PLANE - EYE.z, 0, 1, 0);

	// Set up lighting
	GLfloat lightPosition[] = { 40.0, 40.0, 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// Import texture
	balltexture.readBMPFile("12-grunge-metal-texture.bmp");
	balltexture.setTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Set up cannon.
	xroll = yroll = 0;
	cannonLocation = { 0, 0, 0 };
	cannonMovingBack = cannonMovingForward = 0;
	cannon = gluNewQuadric();
	gluQuadricTexture(cannon, GLU_TRUE);
	gluQuadricDrawStyle(cannon, GLU_FILL);

	initTargets();
	score = 0;
}

// Draws all of the spheres currently on screen.
void drawSpheres() {
	for (list<struct sphere>::iterator i = spheres.begin(); i != spheres.end(); i++) {
		glPushMatrix();
		glTranslated(i->center.x, i->center.y, i->center.z);
		glRotated(i->xroll, 1, 0, 0);
		gluSphere(i->quad, i->radius, SLICES, STACKS);
		glPopMatrix();
	}
}

// Draws the cannon.
void drawCannon() {
	glPushMatrix();
	glTranslatef(cannonLocation.x, cannonLocation.y, cannonLocation.z);
	glRotated(xroll, 1, 0, 0);
	glRotated(yroll, 0, 1, 0);
	gluCylinder(cannon, SPHERE_RADIUS, SPHERE_RADIUS, CANNON_BARREL_LENGTH, SLICES, STACKS);
	glutSolidSphere(SPHERE_RADIUS, SLICES, STACKS);
	glPopMatrix();
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	drawSpheres();
	drawCannon();
	
	char s[10];
	sprintf(s, "%d", score);
	drawText(SCREEN_WIDTH - 60, 20, s);
	if (gameOver) {
		drawText((SCREEN_WIDTH / 2) - 40, (SCREEN_HEIGHT / 2) + 15, "Game Over.");//TODO move this out of here and just keep it in the collision with near plane.
		drawText((SCREEN_WIDTH / 2) - 60, (SCREEN_HEIGHT / 2) - 15, "Try again? (Y/N)");
	}
	
	glFlush();
	glutSwapBuffers();
}

// If the cannon has been recently fired this function will move the cannon one unit
void updateCannonLocation() {
	if (cannonMovingBack > 0) {
		float newZ = cannonLocation.z - CANNON_MOVE_BACK_SPEED;
		cannonLocation = { newZ * (cannonLocation.x / cannonLocation.z), newZ * (cannonLocation.y / cannonLocation.z), newZ };
		// If cannon has stopped moving back, start moving it forward.
		if (--cannonMovingBack == 0) {
			cannonMovingForward = (CANNON_MOVEMENT_DISTANCE / CANNON_MOVE_FORWARD_SPEED);
		}
		glutPostRedisplay();

	} else if (cannonMovingForward > 0) {
		float newZ = cannonLocation.z + CANNON_MOVE_FORWARD_SPEED;
		cannonLocation = { newZ * (cannonLocation.x / cannonLocation.z), newZ * (cannonLocation.y / cannonLocation.z), newZ };
		// If the cannon has finished moving, reset it.
		if (--cannonMovingForward == 0) {
			cannonLocation = { 0, 0, 0 };
		}
		glutPostRedisplay();
	}
}

/* Check to see if a sphere is inside the view volume. A sphere is outside the view volume if it is touching the far
 * or near plane. */
bool outsideViewVolume(struct sphere s) {
	if (s.center.z > FAR_PLANE - s.radius) {
		return true;
	} else if (s.center.z - s.radius < EYE.z + NEAR_PLANE) {
		return true;
	}
	return false;
}

// Update each sphere's location
void updateSphereLocation() {
	if (gameOver) return;
	for (list<struct sphere>::iterator i = spheres.begin(); i != spheres.end(); i++) {
		glutPostRedisplay();
		
		i->center = { i->center.x + (i->v.x * BALL_SPEED),
		              i->center.y + (i->v.y * BALL_SPEED),
		              i->center.z + (i->v.z * BALL_SPEED) };
		i->xroll += i->dxroll;

		if ( outsideViewVolume(*i) ) { // Does this sphere need to be removed?
			if(i->v.z < 0) { // This spehere is a target and just hit the near plane
				glutIdleFunc(NULL);
				gameOver = true;
			} else { // This is a cannon ball, remove it from spheres and continue
				i = spheres.erase(i);
			}
		}
		
	}
}

void detectCollisions() {
	for (list<struct sphere>::iterator i = spheres.begin(); i != spheres.end(); i++) {
		for (list<struct sphere>::iterator j = i; ++j != spheres.end(); ) {
			float dist = sqrt(pow(i->center.x - j->center.x, 2) +
			                  pow(i->center.y - j->center.y, 2) +
			                  pow(i->center.z - j->center.z, 2));
			if (dist <= i->radius + j->radius) {
				struct sphere *target;
				// figure out if i is the target or the cannon ball.
				if (i->v.z <= 0) {
					if (j->v.z <= 0) continue; // if j and j are both targets, ignore collision.
					target = &*i;
					spheres.erase(j);
				} else {
					target = &*j;
					i = spheres.erase(i);
				}
				// Reset target
				target->center.z = FAR_PLANE - target->radius;
				getRandPointInViewVolume(&target->center);
				calculateUnitVector(target, EYE);

				score += SCORE_INCREMENTS;
				break;
			}
		}
	}
}

// Moves the cannon and spheres as necessary.
void updateLocation() {
	updateSphereLocation();
	updateCannonLocation();
	detectCollisions();
}

void fireCannon() {
	float x, y, z;
	x = CANNON_BARREL_LENGTH * sin(yroll * M_PI / 180);
	y = CANNON_BARREL_LENGTH * sin(-xroll * M_PI / 180);
	z = CANNON_BARREL_LENGTH * cos(yroll * M_PI / 180);

	GLUquadric *quad = gluNewQuadric();
	gluQuadricTexture(quad, GLU_TRUE);
	gluQuadricDrawStyle(quad, GLU_FILL);
	gluSphere(quad, SPHERE_RADIUS, SLICES, STACKS);

	spheres.push_front({ quad, {x, y, z}, {x, y, z}, SPHERE_RADIUS, 0, BALL_SPIN_SPEED });

	// Begin the cannon recoil.
	z = CANNON_MOVE_BACK_SPEED;
	x = z * tan(yroll * M_PI / 180);
	y = z * tan(xroll * M_PI / 180);
	cannonLocation = { -x, y, -z };
	cannonMovingBack = (CANNON_MOVEMENT_DISTANCE / CANNON_MOVE_BACK_SPEED) - CANNON_MOVE_BACK_SPEED;
}

// Handles the q and spacebar.
void keypress(unsigned char key, int x, int y) {
	if (gameOver) {
		if (key == 'y' || key == 'Y') {
			//glutRemoveOverlay();
			gameOver = false;
			spheres.clear();
			initTargets();
			score = 0;
			glutIdleFunc(updateLocation);
		} else if (key == 'n' || key == 'N') {
			exit(0);
		}
	} else if (key == ' ' && !cannonMovingBack && !cannonMovingForward) {
		fireCannon();
	}
	if (key == 'q') {
		exit(0);
	}
}

// Handles array key presses. Because of openGL gemoetry, xroll and yroll are backwords so controls aren't inverted.
void specialpress(int key, int x, int y) {
	// If the cannon is moving you can't turn it.
	if (cannonMovingBack || cannonMovingForward) {
		return;
	}

	if (key == GLUT_KEY_UP) {
		if (xroll - CANNON_ROTATE_SPEED >= -MAX_CANNON_ROTATION) {
			xroll -= CANNON_ROTATE_SPEED;
		}
	} else if (key == GLUT_KEY_DOWN) {
		if (xroll + CANNON_ROTATE_SPEED <= MAX_CANNON_ROTATION) {
			xroll += CANNON_ROTATE_SPEED;
		}
	} else if (key == GLUT_KEY_RIGHT) {
		if (yroll - CANNON_ROTATE_SPEED >= -MAX_CANNON_ROTATION) {
			yroll -= CANNON_ROTATE_SPEED;
		}
	} else if (key == GLUT_KEY_LEFT) {
		if (yroll + CANNON_ROTATE_SPEED <= MAX_CANNON_ROTATION) {
			yroll += CANNON_ROTATE_SPEED;
		}
	}
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(100, 150);
	glutCreateWindow("Cannon Game");
	glutDisplayFunc(draw);
	glutKeyboardFunc(keypress);
	glutSpecialFunc(specialpress);
	glutIdleFunc(updateLocation);
	init();
	glutMainLoop();
}