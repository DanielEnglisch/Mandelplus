#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>


const int size = 500;
double zoom = 10E-14;
const double dx = -1.315180982097868;
const double dy = 0.073481649996795;
const unsigned int maxIterations = 1000;

const bool enableZoom = false;
const bool useGPU = false;

GLFWwindow* window;

static void draw();

static unsigned int CompileShader(unsigned int type, const std::string& source) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	/* Error checking */
	int res;
	glGetShaderiv(id, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		int len;
		glGetShaderiv(id,GL_INFO_LOG_LENGTH, &len);
		char* message = (char*)alloca(len * sizeof(char));
		glGetShaderInfoLog(id, len, &len, message);
		std::cout << "Error compiling " << (type == GL_VERTEX_SHADER?"vertex":"fragment") << " shader: " << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned int CreateProgram(const std::string& vert, const std::string& frag) {
	unsigned int programId = glCreateProgram();
	unsigned int vShader = CompileShader(GL_VERTEX_SHADER,vert);
	unsigned int fShader = CompileShader(GL_FRAGMENT_SHADER, frag);
	glAttachShader(programId, vShader);
	glAttachShader(programId, fShader);
	glLinkProgram(programId);
	glValidateProgram(programId);
	glDetachShader(programId, vShader);
	glDetachShader(programId, fShader);

	return programId;
}

static double map(double value, double inMin, double inMax, double outMin, double outMax) {
	return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

int main(void)
{

	/* Initialize the library */
	if (!glfwInit()) {
		std::cout << "glfwInit failed!" << std::endl;
		return -1;
	}

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(size, size, "Hello World", NULL, NULL);
	if (!window)
	{
		std::cout << "Window creation failed!" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Init GLEW */
	if (glewInit() != GLEW_OK) {
		std::cout << "glewInit failed!" << std::endl;
		return -1;
	}

	/* Vertex buffer */
	float vertices[12]{
		-1.0,  1.0,
		-1.0, -1.0,
		 1.0, -1.0,
		-1.0,  1.0,
		 1.0,  1.0,
	 	 1.0, -1.0
	};
	unsigned int vertBuffId = 0;
	glGenBuffers(1, &vertBuffId);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffId);
	glBufferData(GL_ARRAY_BUFFER, 12* sizeof(float), vertices, GL_STATIC_DRAW);
	

	/* Shaders */
	std::string vertShader =
		" #version 400\nprecision highp float;\nattribute vec4 vPos;\nvoid main() {\ngl_Position = vPos;\n}";

	std::string fragShader =
		" #version 400\n#extension GL_ARB_gpu_shader_fp64 : enable\nprecision highp float;\nuniform vec2 dimension;\nuniform float minR;\nuniform float maxR;\nuniform float minI;\nuniform float maxI;\nfloat map(float value, float inMin, float inMax, float outMin, float outMax) {\n  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);\n}\nvoid main() {\n  float a = map(gl_FragCoord.x,0.0,dimension.x,minR,maxR);\n  float b = map(gl_FragCoord.y,0.0,dimension.y,minI,maxI);\n  float initialA = a;\n  float initialB = b;\n  float n = 0.0;\n  float maxIterations = 10000.0;\n  const int maxIterations_int = 10000;\n  for(int i = 0; i < maxIterations_int;i++){\n    float newA = a*a - b*b;\n    float newB = 2.0*a*b;\n    a = initialA + newA;\n    b = initialB + newB;\n    if(abs(a+b)>4.0)\n      break;\n    n += 1.0;\n  }\n  if(n >= maxIterations)\n    discard;\n  else{\n    float normalized =  map(n, 0.0, maxIterations,0.0,1.0);\n    float bri = map(sqrt(normalized*50.0),0.0,1.0,0.0,1.0);\n    gl_FragColor = vec4(\n      map(bri*n*2.4,0.0,200.0,0.0,1.0),\n      map(bri*n,0.0,130.0,0.0,1.0),\n      map(bri, 0.0,3.0,0.0,1.0),\n      1.0);\n  }\n}";

	unsigned int programId = CreateProgram(vertShader, fragShader);
	if(useGPU)
		glUseProgram(programId);

	/* Set Vertex position attribute */
	GLint vertPos = glGetAttribLocation(programId, "vPos");
	glEnableVertexAttribArray(vertPos);
	glVertexAttribPointer(vertPos, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

	

	/* Get Uniform locations */
	GLint minI = glGetUniformLocation(programId, "minI");
	GLint maxI = glGetUniformLocation(programId, "maxI");
	GLint minR = glGetUniformLocation(programId, "minR");
	GLint maxR = glGetUniformLocation(programId, "maxR");
	GLint dimension = glGetUniformLocation(programId, "dimension");
	
	draw();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		/* Set uniforms */
		glUniform1f(minI, -zoom + dx);
		glUniform1f(maxI, zoom + dx);
		glUniform1f(minR, -zoom + dy);
		glUniform1f(maxR, zoom + dy);
		glUniform2f(dimension, size, size);

		if (useGPU) {
			/* Draw call */
			glDrawArrays(GL_TRIANGLES, 0, 6);
			/* Swap front and back buffers */
			glfwSwapBuffers(window);
		}
		
		
	
		

		/* Poll for and process events */
		glfwPollEvents();

		if(enableZoom)
			zoom /= 5;
	}

	glfwTerminate();
	return 0;
}


static void draw() {


		char* data = new char[size*size * 3];

		for (unsigned int x = 0; x < size; x++) {
			for (unsigned int y = 0; y < size; y++) {

				unsigned int i = (x + y * size) * 3;

				// Map pixel position between minR and maxR
				double a = map(x, 0, size, -zoom, zoom);
				double b = map(y, 0, size, -zoom, zoom);
				a += dx;
				b += dy;
				unsigned int n = 0;
				double initialA = a;
				double initialB = b;

				// Iterate
				for (n = 0; n < maxIterations; n++) {
					// Apply mandelbrot formula
					double newA = a * a - b * b;

					double newB = 2 * a*b;

					a = initialA + newA;
					b = initialB + newB;

					// If it gets towards infinity
					if (abs(a + b) > 2)
						break;
				}

				// Map iterations to brightness
				// Normalize
				double brightness = map(n, 0, maxIterations, 0, 1);


				// Map to sqrt(brightness) function
				brightness = map(sqrt(brightness), 0, 1, 0, 1);

				// Ignore infinite values
				if (n == maxIterations) {
					data[i + 0] = 0;
					data[i + 1] = 0;
					data[i + 2] = 0;
				}
					


				data[i + 0] = (int)fmod(brightness / 2 + n, 255);
				data[i + 1] = (int)fmod(brightness * 3 - n, 255);
				data[i + 2] = (int)fmod(2 * brightness + (3 * n - maxIterations), 255);

				


			}
			
			if (x % (size / 4) == 0)
			{
				glDrawPixels(size, size, GL_RGB, GL_UNSIGNED_BYTE, data);
				glfwSwapBuffers(window);
			}


		}

		std::cout << "Calculation done!" << std::endl;
		glDrawPixels(size, size, GL_RGB, GL_UNSIGNED_BYTE, data);
		glfwSwapBuffers(window);
		delete[] data;
	
}