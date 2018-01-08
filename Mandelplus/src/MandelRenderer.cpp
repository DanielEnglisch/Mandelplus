#include "MandelRenderer.h"

Color MandelbrotRenderer::getColor(const int i, const double r, const double c) {
	const double size{ sqrt(r * r + c * c) };
	const double smoothed{ log(log(size) * ONE_OVER_LOG2) * ONE_OVER_LOG2 };
	const int colorI{ (int)(sqrt(i + 1 - smoothed) * 256) % 512 };
	unsigned int cr, cg, cb;
	sscanf_s(hex[colorI].c_str(), "%02x%02x%02x", &cr, &cg, &cb);
	return Color{ (cr),(cg),(cb) };
}

ManVal MandelbrotRenderer::getMandelbrotValue(const int x, const int y) {

	// Map pixel position between minR and maxR
	double a{ map(x, 0, width, -zoom, zoom) + dx };
	double b{ map(y, 0, height, -zoom, zoom) + dy };

	const double initialA{ a };
	const double initialB{ b };

	unsigned int n{ 0 };

	// Iterate
	for (n = 0; n < maxIterations; n++) {
		// Apply mandelbrot formula
		double newA{ a * a - b * b };

		double newB{ 2 * a*b };

		a = initialA + newA;
		b = initialB + newB;

		// If it gets towards infinity
		if (abs(a + b) > 2)
			break;
	}

	return ManVal{ a,b,n };

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
		d[i] = data[i].r;
	}
	return d;
}

double* MandelbrotRenderer::cloneImaginaryData() {
	double* d = new double[numPixels];
	for (unsigned int i{ 0 }; i < numPixels; ++i) {
		d[i] = data[i].c;
	}
	return d;
}

char* MandelbrotRenderer::cloneRGB() {
	char* d = new char[numPixels*3];
	std::memcpy(d, rgbBuffer, sizeof(char)*numPixels*3);
	return d;
}
