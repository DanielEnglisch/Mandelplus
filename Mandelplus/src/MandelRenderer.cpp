#include "MandelRenderer.h"

Color MandelbrotRenderer::getColor(const unsigned int i, mpfr_t  r, mpfr_t  c) {

	//return Color{i%255,i*2%255,i*3%255};

	mpfr_t size,tmp1;
	mpfr_init2(size, 128);
	mpfr_init2(tmp1, 128);

	mpfr_mul(size, r, r, MPFR_RNDU);
	mpfr_mul(tmp1, c,c, MPFR_RNDD);

	mpfr_add(size, size, tmp1, MPFR_RNDU);
	


	//const double smoothed{ log(log(size) * ONE_OVER_LOG2) * ONE_OVER_LOG2 };
	mpfr_log(size, size, MPFR_RNDU);
	mpfr_mul_d(size, size, ONE_OVER_LOG2, MPFR_RNDU);
	mpfr_log(size, size, MPFR_RNDU);
	mpfr_mul_d(size, size, ONE_OVER_LOG2, MPFR_RNDU);
	mpfr_mul_d(size, size, -1.0, MPFR_RNDU);
	mpfr_add_ui(size, size, (i+1), MPFR_RNDU);
	mpfr_sqrt(size, size, MPFR_RNDU);
	mpfr_mul_ui(size, size, 256, MPFR_RNDU);
	mpfr_set_d(tmp1, 512.0, MPFR_RNDU);
	mpfr_fmod(size, size, tmp1, MPFR_RNDU);

	const unsigned int colorI{ mpfr_get_ui(size, MPFR_RNDU)};


	unsigned int cr, cg, cb;
	sscanf_s(hex[colorI].c_str(), "%02x%02x%02x", &cr, &cg, &cb);
	mpfr_clear(size);
	mpfr_clear(tmp1);
	return Color{ (cr),(cg),(cb) };
}

ManVal MandelbrotRenderer::getMandelbrotValue(const int x, const int y) {

#define PREC 128


	mpfr_t a, b;
	mpfr_init2(a, PREC);
	mpfr_init2(b, PREC);
	mpfr_set_d(a, map(x, 0, width, -zoom, zoom) + dx, MPFR_RNDD);
	mpfr_set_d(b, map(y, 0, height, -zoom, zoom) + dy, MPFR_RNDD);


	// Map pixel position between minR and maxR
	//double a{ map(x, 0, width, -zoom, zoom) + dx };
	//double b{ map(y, 0, height, -zoom, zoom) + dy };


	mpfr_t initialA, initialB;
	//const double initialA{ a };
	//const double initialB{ b };
	mpfr_init2(initialA, PREC);
	mpfr_init2(initialB, PREC);
	mpfr_set(initialA, a, MPFR_RNDD);
	mpfr_set(initialB, b, MPFR_RNDD);

	unsigned int n{ 0 };

	// Iterate
	for (n = 0; n < maxIterations; n++) {
		// Apply mandelbrot formula
		mpfr_t newA, newB, tmp1;
		mpfr_init2(newA, PREC);
		mpfr_init2(newB, PREC);
		mpfr_init2(tmp1, PREC);

		mpfr_mul(newA, a, a, MPFR_RNDU);
		mpfr_mul(tmp1, b, b, MPFR_RNDU);
		mpfr_sub(newA, newA, tmp1, MPFR_RNDU);

		mpfr_mul(newB, a, b, MPFR_RNDU);
		mpfr_mul_d(newB, newB, 2.0, MPFR_RNDU);

		mpfr_add(tmp1, initialA, newA, MPFR_RNDU);
		mpfr_set(a, tmp1, MPFR_RNDU);

		mpfr_add(tmp1, initialB, newB, MPFR_RNDU);
		mpfr_set(b, tmp1, MPFR_RNDU);

		mpfr_add(tmp1, a, b, MPFR_RNDU);
		mpfr_abs(tmp1, tmp1, MPFR_RNDU);


		// If it gets towards infinity
		if (mpfr_cmp_ui(tmp1,2.0) > 0)
			break;

		mpfr_clear(newB);
		mpfr_clear(newA);
		mpfr_clear(tmp1);

	}

	//mpfr_clear(a);
	//mpfr_clear(b);
	mpfr_clear(initialA);
	mpfr_clear(initialB);

	return ManVal{ a, b, n };

}

void  MandelbrotRenderer::exportPPM() const {

	FILE *fp;
	errno_t err;

	if ((err = fopen_s(&fp, "export.ppm", "wb")) != 0) {
		std::cerr << "Error opening file!" << std::endl;
	}
	else {
		(void)fprintf(fp, "P6\n%d %d\n255\n", width, height);
		std::cout << "Writing " << sizeof(char)*numPixels * 3 << " Bytes..." << std::endl;
		fwrite(rgbBuffer, sizeof(char), numPixels * 3, fp);
		(void)fclose(fp);
		std::cout << "Exported image: export.ppm!" << std::endl;
	}
}

void MandelbrotRenderer::construct(const unsigned int minWidth, const unsigned int maxWidth, const unsigned int minHeight, const unsigned int maxHeight, ManVal* data, const bool log) {

	for (unsigned int x{ minWidth }; x < maxWidth; x++) {
		for (unsigned int y{ minHeight }; y < maxHeight; y++) {
			const unsigned int i{ (x + y * width) };
			const ManVal v{ getMandelbrotValue(x, y) };
			data[i] = v;
		}
		if (log)
			std::cout << ((x - minWidth) / (double)(maxWidth - minWidth)) * 100 << "%" << std::endl;

	}

}

void MandelbrotRenderer::color() {
	if (rgbBuffer != nullptr) {
		delete[] rgbBuffer;
		rgbBuffer = new char[numPixels * 3];
	}

	std::cout << "=== Coloring... ===" << std::endl;


	for (unsigned int x{ 0 }; x < width; x++) {
		for (unsigned int y{ 0 }; y < height; y++) {

			const unsigned int dataIndex{ (x + y * width) };
			const unsigned int i{ dataIndex * 3 };

			ManVal v{ data[dataIndex] };
			Color c{ getColor(v.i,v.r,v.c) };

			rgbBuffer[i + 0] = c.r;
			rgbBuffer[i + 1] = c.g;
			rgbBuffer[i + 2] = c.b;

		}
		//std::cout << ((x * width) / (double)numPixels)*100 << "%" << std::endl;
	}

}

void MandelbrotRenderer::generate() {
	if (data != nullptr) {
		delete[] data;
		data = new ManVal[numPixels];
	}

	std::cout << "Rendering  " << width << "x" << height << " with " << maxIterations << " iterations" << std::endl;
	std::cout << "Zoom:" << zoom << std::endl;
	std::cout << "Dx:" << dx << std::endl;
	std::cout << "Dy:" << dy << std::endl;

	const clock_t begin{ clock() };
	std::cout << "Rendering with 4 threads..." << std::endl;

	threads.push_back(std::thread(&MandelbrotRenderer::construct, this, 0, width / 2, 0, height / 2, data, false));
	threads.push_back(std::thread(&MandelbrotRenderer::construct, this, width / 2, width, 0, height / 2, data, false));
	threads.push_back(std::thread(&MandelbrotRenderer::construct, this, 0, width / 2, height / 2, height, data, false));
	construct(width / 2, width, height / 2, height, data, true);


	for (unsigned int i{ 0 }; i < threads.size(); ++i)
	{
		if (threads[i].joinable())
			threads.at(i).join();
	}

	std::cout << "Done! Took " << (double(clock() - begin) / CLOCKS_PER_SEC) << "s" << std::endl;

}


void MandelbrotRenderer::show() {

	/* Initialize the library */
	if (!glfwInit()) {
		std::cout << "glfwInit failed!" << std::endl;
	}

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Mandelplus", NULL, NULL);
	if (!window)
	{
		std::cout << "Window creation failed!" << std::endl;
		glfwTerminate();
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Init GLEW */
	if (glewInit() != GLEW_OK) {
		std::cout << "glewInit failed!" << std::endl;
	}

	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, rgbBuffer);
	glfwSwapBuffers(window);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
		glfwPollEvents();

	glfwTerminate();
}

ManVal* MandelbrotRenderer::cloneData() {
	ManVal* d = new ManVal[numPixels];
	std::memcpy(d, data, sizeof(ManVal)*numPixels);
	return d;
}

unsigned int* MandelbrotRenderer::cloneIterationData() {
	unsigned int* d = new unsigned int[numPixels];
	for (unsigned int i{ 0 }; i < numPixels; ++i) {
		d[i] = data[i].i;
	}
	return d;
}

double* MandelbrotRenderer::cloneRealData() {
	double* d = new double[numPixels];
	for (unsigned int i{ 0 }; i < numPixels; ++i) {
		d[i] = mpfr_get_d(data[i].r, MPFR_RNDU);
	}
	return d;
}

double* MandelbrotRenderer::cloneImaginaryData() {
	double* d = new double[numPixels];
	for (unsigned int i{ 0 }; i < numPixels; ++i) {
		d[i] = mpfr_get_d(data[i].c, MPFR_RNDU);
	}
	return d;
}

char* MandelbrotRenderer::cloneRGB() {
	char* d = new char[numPixels*3];
	std::memcpy(d, rgbBuffer, sizeof(char)*numPixels*3);
	return d;
}
