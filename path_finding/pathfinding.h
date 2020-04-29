#pragma once

// Main
int main(int argc, char ** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(2 * (512 + 4), 2 * (256 + 2));
	glutCreateWindow("Pathfinding Demo");
	glutDisplayFunc(draw);
	if (!init()) return -1;
	glutMainLoop();
	delete inputData;
	delete outputData;
	return 0;
}
