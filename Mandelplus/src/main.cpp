#include "MandelRenderer.h"

int main(int argc, char* argv[]){

	/*  cloverleaf */
	const double dx = -0.04524074130409;
	const double dy = 0.9868162207157838;
	const double zoom = 5.0E-12;
	const unsigned int iterations = 2000;
	const unsigned int size = 1000;

	MandelbrotRenderer* r1{ new MandelbrotRenderer{size, size, iterations, zoom, dx,dy } };
	r1->generate();
	r1->color();
	r1->exportPPM();
	r1->show();

 	delete r1;

	return 0;

}
