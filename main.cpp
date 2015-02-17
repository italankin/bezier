#include <windows.h> 								// for ms windows
#include <GL\freeglut.h>							// glut
#include <vector>
#include "bezier.h"									// bezier calculations and classes
#include "files.h"									// tools for reading files and converting into objects
#include "utils.h"									// mouse class and other tools

const double FIELD_SIZE = 100.0; 					// grid size
const double ROTATION_STEP = 2.0; 					// rotation step (degrees) used for rotate tool
const double TRANSLATE_STEP = 20.0; 				// translate step used for keyboard arrows
const int WINDOW_WIDTH = 800; 						// default windows width
const int WINDOW_HEIGHT = 600; 						// and height

enum MENU_TYPE { 									// items list for glut context menu
	MENU_AXES,
	MENU_FULLSCREEN,
	MENU_GRID,
	MENU_RELOAD
};

GLfloat xRotation = 10;   							// default
GLfloat yRotation = -10; 							// scene
GLfloat zRotation = 0;   							// rotation

GLfloat xTranslate = 0.0;							// default
GLfloat yTranslate = 0.0;							// scene
GLfloat zTranslate = -650.;							// translation

bool drawAxis = true;								// show axis switch
bool drawGrid = true;								// show grid switch
bool fullscreen = false;							// enable fullscreen switch

Mouse mouse;										// mouse object used for scene manipulations with mouse

vector<surface*> curves;							// vector array of objects drawn in scene (read from file)

char* filename;										// input file name



// draw a simple bezier curve
void drawSurface(surface1d s) {
	Point *p, *l = s.anchors[0];

	glColor3f(0.8f, 0.0, 0.8f);

	for (int i = 1; i < s.dimen_n; i++) {
		p = s.anchors[i];
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(p->x, p->y, p->z);
		glVertex3f(l->x, l->y, l->z);
		glEnd();
		l = p;
	}
}

// draw bezier surface
void drawSurface(surface2d s) {
	Point *p1, *p2, *l1, *l2;

	bool nl;

	glColor3f(0.0f, 0.7f, 0.9f);

	for (int u = 0; u < s.dimen_n; u++) {
		nl = true;									// start a new line, if point is on the edge
													// (means there are no other points to connect with)
		for (int v = 0; v < s.dimen_m; v++) {
			p1 = s.anchors[u][v];					// points used to draw horizontal
			p2 = s.anchors[v][u];					// and vertical lines
			glBegin(GL_LINES);
			glVertex3f(p1->x, p1->y, p1->z);
			if (!nl) {
				glVertex3f(l1->x, l1->y, l1->z);
			}
			glEnd();
			glBegin(GL_LINES);
			glVertex3f(p2->x, p2->y, p2->z);
			if (!nl) {
				glVertex3f(l2->x, l2->y, l2->z);
			}
			glEnd();
			l1 = p1;
			l2 = p2;
			nl = false;
		} // END v
	} // END u
}

// draw a string in (x;y)
void renderString(string s, int x, int y) {
	glRasterPos2i(x, y);
	void * font = GLUT_BITMAP_8_BY_13;
	for (string::iterator i = s.begin(); i != s.end(); ++i) {
		char c = *i;
		glutBitmapCharacter(font, c);
	}
}

// main render loop
void render() {
	glMatrixMode(GL_MODELVIEW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(xTranslate, yTranslate, zTranslate); // translate scene

	glRotatef(xRotation, 1.0, 0.0, 0.0);			// rotate
	glRotatef(yRotation, 0.0, 1.0, 0.0);			// scene
	glRotatef(zRotation, 0.0, 0.0, 1.0);			//

	// draw grid lines, if enabled
	if (drawGrid) {
		int lines = FIELD_SIZE;
		for (int i = -lines; i < lines; i++) {
			if (i == 0 && drawAxis)					// do not draw grid lines where zero axis are
				continue;
			glColor3f(0.95f, 0.95f, 0.95f);			// light grey
			glBegin(GL_LINES);
			glVertex3f(i, 0.0f, -FIELD_SIZE);
			glVertex3f(i, 0.0f, FIELD_SIZE);
			glEnd();
			glBegin(GL_LINES);
			glVertex3f(-FIELD_SIZE, 0.0f, i);
			glVertex3f(FIELD_SIZE, 0.0f, i);
			glEnd();
		}
	}

	if (drawAxis) {
		// y axis (green)
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, FIELD_SIZE, 0.0f);
		glEnd();

		glColor3f(0.8f, 1.0f, 0.8f);
		glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, -FIELD_SIZE, 0.0f);
		glEnd();

		// x axis (red)
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex3f(-FIELD_SIZE, 0.0f, 0.0f);
		glVertex3f(FIELD_SIZE, 0.0f, 0.0f);
		glEnd();

		// z axis (blue)
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, -FIELD_SIZE);
		glVertex3f(0.0f, 0.0f, FIELD_SIZE);
		glEnd();
	}

	// draw bezier objects
	for (unsigned int i = 0; i < curves.size(); i++) {
		if (curves[i]->type == SURFACE1D) {
			surface1d* sd = (surface1d*) curves[i];
			drawSurface(*sd);
		} else {
			surface2d* sd = (surface2d*) curves[i];
			drawSurface(*sd);
		}
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0,
			glutGet(GLUT_WINDOW_HEIGHT));
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	// OSD info texts
	glColor3f(0.0f, 0.0f, 0.0f);
	string s = "divs=" + int_to_string(divisions);
	renderString(s, 10, 10);
	s = "pgup/pgdn";
	renderString(s, 10, 30);
	s = "rotation=(" + int_to_string(xRotation) + "," + int_to_string(yRotation)
			+ "," + int_to_string(zRotation) + ")";
	renderString(s, 110, 10);
	s = "arrow keys";
	renderString(s, 110, 30);
	s = "translation=(" + int_to_string(xTranslate) + ","
			+ int_to_string(yTranslate) + "," + int_to_string(zTranslate) + ")";
	renderString(s, 280, 10);
	s = "ctrl+arrow keys";
	renderString(s, 280, 30);
	s = "fullscreen=" + (string) (fullscreen ? "1" : "0");
	renderString(s, 470, 10);
	s = "f";
	renderString(s, 470, 30);
	s = "grid=" + (string) (drawGrid ? "1" : "0");
	renderString(s, 580, 10);
	s = "g";
	renderString(s, 580, 30);
	s = "axis=" + (string) (drawAxis ? "1" : "0");
	renderString(s, 650, 10);
	s = "a";
	renderString(s, 650, 30);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glFlush();
	glutSwapBuffers();
}

//
void arrowKeyPressed(int key, int x, int y) {
	int mod = glutGetModifiers();
	switch (key) {
		
	case GLUT_KEY_LEFT:
		switch (mod) {
		case GLUT_ACTIVE_CTRL:
			xTranslate += TRANSLATE_STEP / 100.0;
			break;
		default:
			yRotation += ROTATION_STEP;
			if (yRotation > 360.0f) {
				yRotation -= 360;
			}
		}
		break;
	//
	
	case GLUT_KEY_RIGHT:
		switch (mod) {
		case GLUT_ACTIVE_CTRL:
			xTranslate -= TRANSLATE_STEP / 100.0;
			break;
		default:
			yRotation -= ROTATION_STEP;
			if (yRotation > 360.0f) {
				yRotation -= 360;
			}
		}
		break;
	//
	
	case GLUT_KEY_UP:
		switch (mod) {
		case GLUT_ACTIVE_CTRL:
			yTranslate -= TRANSLATE_STEP / 100.0;
			break;
		default:
			xRotation += ROTATION_STEP;
			if (xRotation > 360.0f) {
				xRotation -= 360;
			}
		}
		break;
	//

	case GLUT_KEY_DOWN:
		switch (mod) {
		case GLUT_ACTIVE_CTRL:
			yTranslate += TRANSLATE_STEP / 100.0;
			break;
		default:
			xRotation -= ROTATION_STEP;
			if (xRotation > 360.0f) {
				xRotation -= 360;
			}
		}
		break;
	//

	case GLUT_KEY_PAGE_DOWN:
		if (divisions > 4) {
			divisions -= 2;
		}
		break;
	//

	case GLUT_KEY_PAGE_UP:
		if (divisions < (DIMEN_BUFFER - 2)) {
			divisions += 2;
		}
		break;
	//

	} // end switch

	render();
}

//
void mouseMove(int x, int y) {
	mouse.set(x, y);
	if (mouse.active) {
		switch (mouse.button) {
		case GLUT_MIDDLE_BUTTON:
			xTranslate += mouse.dx() / 75.0;
			yTranslate -= mouse.dy() / 75.0;
			break;
		//
		
		case GLUT_LEFT_BUTTON:
			xRotation += mouse.dy() / 10.0;
			if (xRotation > 360.0f) {
				xRotation -= 360;
			}
			yRotation += mouse.dx() / 10.0;
			if (yRotation > 360.0f) {
				yRotation -= 360;
			}
			break;
		//
		
		} // END switch

		render();

	} // END mouse.active
}

//
void mousePress(int button, int state, int x, int y) {
	switch (button) {
	case 3:
		zTranslate += TRANSLATE_STEP;
		render();
		break;
	//

	case 4:
		zTranslate -= TRANSLATE_STEP;
		render();
		break;
	//

	default:
		mouse.set(x, y);
		mouse.button = button;
		mouse.state = state;
		mouse.active = state == GLUT_DOWN;
	//

	} // END switch
	
}

//
void keyPressed(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		drawAxis = !drawAxis;
		break;
	//

	case 'g':
		drawGrid = !drawGrid;
		break;
	//

	case 'f':
		fullscreen = !fullscreen;
		if (fullscreen) {
			glutFullScreen();
		} else {
			glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
		}
		break;
	//

	case 'd':
		xRotation = 10;
		yRotation = -10;
		xTranslate = 0.0;
		yTranslate = 0.0;
		zTranslate = -400.0;
		break;
	//

	} // END switch

	render();
}

//
void reshape(int x, int y) {
	if (y == 0 || x == 0)
		return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(1.0, (GLdouble) x / (GLdouble) y, 1.5, 20000.0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, x, y);
}

//
void menu(int item) {
	switch (item) {
	case MENU_AXES:
		drawAxis = !drawAxis;
		render();
		break;
	//

	case MENU_GRID:
		drawGrid = !drawGrid;
		render();
		break;
	//

	case MENU_FULLSCREEN:
		fullscreen = !fullscreen;
		if (fullscreen) {
			glutFullScreen();
		} else {
			glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
		}
		break;
	//

	case MENU_RELOAD:
		curves = read_file(filename);
		render();
		break;
	//

	} // END switch

	glutPostRedisplay();

}

int main(int argc, char** argv) {

	if (argc > 1) {
		filename = argv[1];
		// load bezier objects from file
		curves = read_file(filename);				
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - WINDOW_WIDTH) / 2,
			(glutGet(GLUT_SCREEN_HEIGHT) - WINDOW_HEIGHT) / 2);
	glutCreateWindow("bezier");
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	glClearColor(255, 255, 255, 0);
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutSpecialFunc(arrowKeyPressed);
	glutKeyboardFunc(keyPressed);
	glutMotionFunc(mouseMove);
	glutMouseFunc(mousePress);

	glutCreateMenu(menu);

	// add menu items
	glutAddMenuEntry("Toggle axis", MENU_AXES);
	glutAddMenuEntry("Toggle grid", MENU_GRID);
	glutAddMenuEntry("Toggle fullscreen", MENU_FULLSCREEN);
	glutAddMenuEntry("Reload file", MENU_RELOAD);

	// associate a mouse button with menu
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();

	mouse = Mouse();

	return 0;
}
