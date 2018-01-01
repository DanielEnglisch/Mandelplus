#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

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

static unsigned int CreateShader(const std::string& vert, const std::string& frag) {
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

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit()) {
		std::cout << "glfwInit failed!" << std::endl;
		return -1;
	}

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(500, 500, "Hello World", NULL, NULL);
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
		"precision highp float;\nlayout(location=0)in vec4 vPos;\nvoid main() {\ngl_Position = vPos;\n}";

	std::string fragShader =
		"precision highp float;\nuniform vec2 dimension;\nuniform float minR;\nuniform float maxR;\nuniform float minI;\nuniform float maxI;\nfloat map(float value, float inMin, float inMax, float outMin, float outMax) {\n  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);\n}\nvoid main() {\n  float a = map(gl_FragCoord.x,0.0,dimension.x,minR,maxR);\n  float b = map(gl_FragCoord.y,0.0,dimension.y,minI,maxI);\n  float initialA = a;\n  float initialB = b;\n  float n = 0.0;\n  float maxIterations = 10000.0;\n  const int maxIterations_int = 10000;\n  for(int i = 0; i < maxIterations_int;i++){\n    float newA = a*a - b*b;\n    float newB = 2.0*a*b;\n    a = initialA + newA;\n    b = initialB + newB;\n    if(abs(a+b)>4.0)\n      break;\n    n += 1.0;\n  }\n  if(n >= maxIterations)\n    discard;\n  else{\n    float normalized =  map(n, 0.0, maxIterations,0.0,1.0);\n    float bri = map(sqrt(normalized*50.0),0.0,1.0,0.0,1.0);\n    gl_FragColor = vec4(\n      map(bri*n*2.4,0.0,200.0,0.0,1.0),\n      map(bri*n,0.0,130.0,0.0,1.0),\n      map(bri, 0.0,3.0,0.0,1.0),\n      1.0);\n  }\n}";

	unsigned int programId = CreateShader(vertShader, fragShader);
	glUseProgram(programId);

	// Set Vertex position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

	// Set uniforms
	GLint minI = glGetUniformLocation(programId, "minI");
	GLint maxI = glGetUniformLocation(programId, "maxI");
	GLint minR = glGetUniformLocation(programId, "minR");
	GLint maxR = glGetUniformLocation(programId, "maxR");
	GLint dimension = glGetUniformLocation(programId, "dimension");
	glUniform1f(minI, -6.5E-4 + 0.1127);
	glUniform1f(maxI, 6.5E-4 + 0.1127);
	glUniform1f(minR, -6.5E-4 - 0.7453);
	glUniform1f(maxR, 6.5E-4 - 0.7453);
	glUniform2f(dimension, 500, 500);


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		/* Draw call */
		glDrawArrays(GL_TRIANGLES,0,6);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}