#include"OSPlib.h"

#define WIDTH	640
#define HEIGHT	480

#define SIZE	100

void cube() {
	glBegin(GL_QUADS);
		glColor4ub(255,0,0,0x80); //red
			glVertex3d(1,1,1);
			glVertex3d(1,1,-1);
			glVertex3d(-1,1,-1);
			glVertex3d(-1,1,1);

		glColor4ub(0,255,0,0x80); //green
			glVertex3d(1,-1,1);
			glVertex3d(1,-1,-1);
			glVertex3d(1,1,-1);
			glVertex3d(1,1,1);

		glColor4ub(0,0,255,0x80); //blue
			glVertex3d(-1,-1,1);
			glVertex3d(-1,-1,-1);
			glVertex3d(1,-1,-1);
			glVertex3d(1,-1,1);

		glColor4ub(255,255,0,0x80); //yellow
			glVertex3d(-1,1,1);
			glVertex3d(-1,1,-1);
			glVertex3d(-1,-1,-1);
			glVertex3d(-1,-1,1);

		glColor4ub(0,255,255,0x80); //cyan
			glVertex3d(1,1,-1);
			glVertex3d(1,-1,-1);
			glVertex3d(-1,-1,-1);
			glVertex3d(-1,1,-1);

		glColor4ub(255,0,255,0x80); //magenta
			glVertex3d(1,-1,1);
			glVertex3d(1,1,1);
			glVertex3d(-1,1,1);
			glVertex3d(-1,-1,1);
	glEnd();
}

int main(int argc, char *argv[]) {
	OSPdisplay *dpy;
	OSPwindow *wnd;
	OSPscene *scn;
	OSPobj *trig;
	
	int x;
	int y;
	int z;
	int run = 0x01;
	double green = 0;

	/* Create objects */

	wnd = OSPWnd(0, "OSPOOC Demo", 0, 0, WIDTH, HEIGHT, 0x00000000);
	dpy = OSPDpyOf(wnd); /* Save display here */

	if(!wnd) {
		OSPFre(0);
		return 0;
	}

	scn = OSPScn(wnd, 0, 0, WIDTH, HEIGHT, 0x00000000);

	if(!scn) {
		OSPFre(0);
		return 0;
	}

	while(run) {
		int timestat = 16;
		trig = OSPWte(&timestat);

		if(timestat) {
			/* Since dpy has been destroyed, and window too,
				we use here previously saved dpy.
				&OSPDpyOf(wnd)->_obj would segfault here */

			if(trig == &dpy->_obj) {
				run = 0;
			}
		}
		else{
			if(x > 359) x = 0;
			else x++;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			glViewport(0, 0, WIDTH, HEIGHT);
			GLfloat h = (GLfloat) HEIGHT / (GLfloat) WIDTH;

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glFrustum(-1, 1, -h, h, 1, 100000);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			OSPRun(&scn->_obj, OSPSCN_LOOK, (GLdouble) 0, (GLdouble) 3, (GLdouble) -5,
											(GLdouble) 0, (GLdouble) 0, (GLdouble) 0,
											(GLdouble) 0, (GLdouble) 1, (GLdouble) 0);

//			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glPushMatrix();
				glTranslated(3, 0, 0);
				glRotated(x << 1, 0, 1, 0);
				glRotated(x, 1, 0, 1);
				cube();
			glPopMatrix();

			glPushMatrix();
				glTranslated(-3, 0, 0);
				glRotated(x, 0, 1, 0);
				glRotated(x << 1, 1, 0, 1);
				cube();
			glPopMatrix();

			glPushMatrix();
				glRotated(x << 1, 0, 1, 0);
				glRotated(x << 1, 1, 0, 1);
				cube();
			glPopMatrix();

			glFlush();

			OSPRun(&scn->_obj, OSPWND_SWAP);
		}
	}

	OSPFre(0);
	return 0;
}

