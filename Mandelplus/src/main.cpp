#include "MandelRenderer.h"



int main(void)
{
	/* Spiral */
	double dx = 0.2549870375144766;
	double dy = -0.0005679790528465;
	double zoom = 10E-14;
	
	/* Deep Swirl */
	//double dx = -1.315180982097868;
	//double dy = 0.073481649996795;

	/* Spider */
	//double dx = -0.16070135;
	//double dy = 1.0375665;
	//double zoom = 10E-7;

	MandelbrotRenderer* r1{ new MandelbrotRenderer{ 1000,1000,5000, zoom, dx,dy } };
	r1->generate();
	r1->color();
	r1->exportPPM();
	r1->show();
	delete r1;

}
