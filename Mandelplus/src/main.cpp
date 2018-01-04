#include "MandelRenderer.h"
#include <string>     // std::string, std::stod

int main(int argc, char* argv[]){

	// w,h,dx,dy,z,iter
	if (argc != 7) {
		std::cout << "Usage: " << argv[0] << " <width> <height> <dx> dy> <zoom> <iterations>" << std::endl;
		return 1;
	}

	/* Spiral */
	//double dx = 0.2549870375144766;
	//double dy = -0.0005679790528465;
	//double zoom = 10E-14;

	/* Lightnings */
	//double dx = -0.235125;
	//double dy = 0.827215;
	//double zoom = 50E-9;

	/*  cloverleaf */
	//double dx = -0.04524074130409;
	//double dy = 0.9868162207157838;
	//double zoom = 5.0E-12;
	
	/* Deep Swirl */
	//double dx = -1.315180982097868;
	//double dy = 0.073481649996795;
	//double zoom = 10E-7;

	/* Spider */
	//double dx = -0.16070135;
	//double dy = 1.0375665;
	//double zoom = 10E-7;

	const unsigned int width = atoi(argv[1]);
	const unsigned int height = atoi(argv[2]);
	const double dx = std::stod(argv[3]);
	const double dy = std::stod(argv[4]);
	const double zoom = std::stod(argv[5]);
	const unsigned int iterations = atoi(argv[6]);


	MandelbrotRenderer* r1{ new MandelbrotRenderer{ width,height,iterations, zoom, dx,dy } };
	r1->generate();
	r1->color();
	r1->exportPPM();
	r1->show();

 	delete r1;

	return 0;

}
