#include "MandelRenderer.h"

int main(void)
{
	/* Spiral */
	//double dx = 0.2549870375144766;
	//double dy = -0.0005679790528465;
	//double zoom = 10E-14;

	/* Lightnings */
	//double dx = -0.235125;
	//double dy = 0.827215;
	//double zoom = 50E-9;

	/*  cloverleaf */
	double dx = -0.04524074130409;
	double dy = 0.9868162207157838;
	double zoom = 5.0E-12;
	
	/* Deep Swirl */
	//double dx = -1.315180982097868;
	//double dy = 0.073481649996795;
	//double zoom = 10E-7;

	/* Spider */
	//double dx = -0.16070135;
	//double dy = 1.0375665;
	//double zoom = 10E-7;

	MandelbrotRenderer* r1{ new MandelbrotRenderer{ 100,100,1000, zoom, dx,dy } };
	r1->generate();
	r1->color();
	r1->exportPPM();
	r1->show();

	unsigned int* iterations{ r1->cloneIterationData() };
	unsigned int size{ r1->getNumPixels() };
	delete r1;

	for (unsigned int i{ 0 }; i < size; ++i)
		std::cout << iterations[i] << ", ";
	std::cout << std::endl;
	delete[] iterations;
}
