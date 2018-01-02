#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>

/* Structs */
struct Color {
	int r, g, b;
};

struct ManVal {
	double r, c;
	int i;
};

/* Prototypes */
static  double map(double value, double inMin, double inMax, double outMin, double outMax);
static unsigned int CompileShader(unsigned int type, const std::string& source);
static unsigned int CreateProgram(const std::string& vert, const std::string& frag);
static void drawMandelbrot(GLFWwindow* window, const unsigned int size, const double dx, const double dy, const double zoom, const unsigned int maxIterations);
static ManVal getMandelbrotValue(const unsigned int size, const unsigned int x, const unsigned int y, const double dx, const double dy, const double zoom, const unsigned int maxIterations);


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
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		char* message = (char*)alloca(len * sizeof(char));
		glGetShaderInfoLog(id, len, &len, message);
		std::cout << "Error compiling " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: " << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned int CreateProgram(const std::string& vert, const std::string& frag) {
	unsigned int programId = glCreateProgram();
	unsigned int vShader = CompileShader(GL_VERTEX_SHADER, vert);
	unsigned int fShader = CompileShader(GL_FRAGMENT_SHADER, frag);
	glAttachShader(programId, vShader);
	glAttachShader(programId, fShader);
	glLinkProgram(programId);
	glValidateProgram(programId);
	glDetachShader(programId, vShader);
	glDetachShader(programId, fShader);

	return programId;
}


static  double map(double value, double inMin, double inMax, double outMin, double outMax) {
	return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

class MandelbrotRenderer {
private:
	int size{ 100 };
	double dx{ 0 };
	double dy{ 0 };
	int maxIterations{ 100 };
	double zoom{ 2 };
	GLFWwindow* window{ nullptr };
public:
	bool enableZoom{ true };
	bool useGPU{ false };

	MandelbrotRenderer(int size, int maxIterations, double zoom, double dx, double dy)
		:size{ size }, maxIterations{ maxIterations }, zoom{ zoom }, dx{ dx }, dy{ dy }
	{
		std::cout << "Init done!" << std::endl;

	}
	~MandelbrotRenderer() {
		//glfwTerminate();
		//delete window;
	}
	int run() {

		std::cout << "UseGPU: " << useGPU << std::endl;
		std::cout << "enableZoom: " << enableZoom << std::endl;
		std::cout << "Rendering  " << size << "x" << size << " with " << maxIterations << " iterations" << std::endl;

		/* Initialize the library */
		if (!glfwInit()) {
			std::cout << "glfwInit failed!" << std::endl;
			return 0;
		}

		/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(size, size, "Mandelplus", NULL, NULL);
		if (!window)
		{
			std::cout << "Window creation failed!" << std::endl;
			glfwTerminate();
			return 0;
		}

		/* Make the window's context current */
		glfwMakeContextCurrent(window);

		/* Init GLEW */
		if (glewInit() != GLEW_OK) {
			std::cout << "glewInit failed!" << std::endl;
			return 0;
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
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);

		/* Shaders */
		std::string vertShader =
			" #version 400\nprecision highp float;\nattribute vec4 vPos;\nvoid main() {\ngl_Position = vPos;\n}";

		std::string fragShader =
			" #version 400\n#extension GL_ARB_gpu_shader_fp64 : enable\nprecision highp float;\nuniform vec2 dimension;\nuniform float minR;\nuniform float maxR;\nuniform float minI;\nuniform float maxI;\nfloat map(float value, float inMin, float inMax, float outMin, float outMax) {\n  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);\n}\nvoid main() {\n  float a = map(gl_FragCoord.x,0.0,dimension.x,minR,maxR);\n  float b = map(gl_FragCoord.y,0.0,dimension.y,minI,maxI);\n  float initialA = a;\n  float initialB = b;\n  float n = 0.0;\n  float maxIterations = 10000.0;\n  const int maxIterations_int = 10000;\n  for(int i = 0; i < maxIterations_int;i++){\n    float newA = a*a - b*b;\n    float newB = 2.0*a*b;\n    a = initialA + newA;\n    b = initialB + newB;\n    if(abs(a+b)>4.0)\n      break;\n    n += 1.0;\n  }\n  if(n >= maxIterations)\n    discard;\n  else{\n    float normalized =  map(n, 0.0, maxIterations,0.0,1.0);\n    float bri = map(sqrt(normalized*50.0),0.0,1.0,0.0,1.0);\n    gl_FragColor = vec4(\n      map(bri*n*2.4,0.0,200.0,0.0,1.0),\n      map(bri*n,0.0,130.0,0.0,1.0),\n      map(bri, 0.0,3.0,0.0,1.0),\n      1.0);\n  }\n}";

		unsigned int programId = CreateProgram(vertShader, fragShader);
		if (useGPU)
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

		if (!useGPU)
			drawMandelbrot(window, size, dx, dy, zoom, maxIterations);
		std::cout << "Entering loop!" << std::endl;

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
			else {
				if(enableZoom)
					drawMandelbrot(window, size, dx, dy, zoom, maxIterations);
			}


			/* Poll for and process events */
			glfwPollEvents();

			
			if (enableZoom)
				zoom /= 10;

		}

		glfwTerminate();
		return 0;
	}

};


/* Gradient from https://www.strangeplanet.fr/work/gradient-generator/index.php */
std::string hex[512]{
	"000764", "000764", "000865", "000966", "010A67", "010A68", "010B68", "010C69", "020D6A", "020D6B", "020E6C", "020F6C", "03106D", "03116E", "03116F", "031270", "041370", "041471", "041472", "041573", "051674", "051774", "051875", "051876", "061977", "061A78", "061B78", "061B79", "071C7A", "071D7B", "071E7C", "071E7C", "081F7D", "08207E", "08217F", "082280", "092280", "092381", "092482", "092583", "0A2584", "0A2684", "0A2785", "0A2886", "0B2987", "0B2988", "0B2A89", "0B2B89", "0C2C8A", "0C2C8B", "0C2D8C", "0C2E8D", "0D2F8D", "0D2F8E", "0D308F", "0D3190", "0E3291", "0E3391", "0E3392", "0E3493", "0F3594", "0F3695", "0F3695", "0F3796", "103897", "103998", "103A99", "103A99", "113B9A", "113C9B", "113D9C", "113D9D", "123E9D", "123F9E", "12409F", "1241A0", "1341A1", "1342A1", "1343A2", "1344A3", "1444A4", "1445A5", "1446A5", "1447A6", "1547A7", "1548A8", "1549A9", "154AAA", "164BAA", "164BAB", "164CAC", "164DAD", "174EAE", "174EAE", "174FAF", "1750B0", "1851B1", "1852B2", "1852B2", "1853B3", "1954B4", "1955B5", "1955B6", "1956B6", "1A57B7", "1A58B8", "1A58B9", "1A59BA", "1B5ABA", "1B5BBB", "1B5CBC", "1B5CBD", "1C5DBE", "1C5EBE", "1C5FBF", "1C5FC0", "1D60C1", "1D61C2", "1D62C2", "1D63C3", "1E63C4", "1E64C5", "1E65C6", "1E66C6", "1F66C7", "1F67C8", "1F68C9", "1F69CA", "206ACB", "216BCB", "236CCB", "246DCC", "266ECC", "286FCD", "2970CD", "2B72CD", "2C73CE", "2E74CE", "3075CF", "3176CF", "3377CF", "3479D0", "367AD0", "387BD1", "397CD1", "3B7DD1", "3C7ED2", "3E80D2", "4081D3", "4182D3", "4383D3", "4584D4", "4685D4", "4887D5", "4988D5", "4B89D5", "4D8AD6", "4E8BD6", "508CD7", "518ED7", "538FD8", "5590D8", "5691D8", "5892D9", "5993D9", "5B95DA", "5D96DA", "5E97DA", "6098DB", "6199DB", "639ADC", "659CDC", "669DDC", "689EDD", "6A9FDD", "6BA0DE", "6DA1DE", "6EA3DE", "70A4DF", "72A5DF", "73A6E0", "75A7E0", "76A8E0", "78AAE1", "7AABE1", "7BACE2", "7DADE2", "7EAEE2", "80AFE3", "82B1E3", "83B2E4", "85B3E4", "87B4E5", "88B5E5", "8AB6E5", "8BB7E6", "8DB9E6", "8FBAE7", "90BBE7", "92BCE7", "93BDE8", "95BEE8", "97C0E9", "98C1E9", "9AC2E9", "9BC3EA", "9DC4EA", "9FC5EB", "A0C7EB", "A2C8EB", "A3C9EC", "A5CAEC", "A7CBED", "A8CCED", "AACEED", "ACCFEE", "ADD0EE", "AFD1EF", "B0D2EF", "B2D3EF", "B4D5F0", "B5D6F0", "B7D7F1", "B8D8F1", "BAD9F2", "BCDAF2", "BDDCF2", "BFDDF3", "C0DEF3", "C2DFF4", "C4E0F4", "C5E1F4", "C7E3F5", "C8E4F5", "CAE5F6", "CCE6F6", "CDE7F6", "CFE8F7", "D1EAF7", "D2EBF8", "D4ECF8", "D5EDF8", "D7EEF9", "D9EFF9", "DAF1FA", "DCF2FA", "DDF3FA", "DFF4FB", "E1F5FB", "E2F6FC", "E4F8FC", "E5F9FC", "E7FAFD", "E9FBFD", "EAFCFE", "ECFDFE", "EEFFFF", "EEFEFD", "EEFDFB", "EEFDF9", "EEFCF7", "EEFBF5", "EEFBF3", "EEFAF1", "EFF9EF", "EFF9ED", "EFF8EB", "EFF7E9", "EFF7E7", "EFF6E5", "EFF5E3", "EFF5E1", "F0F4DF", "F0F3DD", "F0F3DB", "F0F2D9", "F0F1D7", "F0F1D5", "F0F0D3", "F1EFD1", "F1EFCF", "F1EECD", "F1EDCB", "F1EDC9", "F1ECC7", "F1EBC5", "F1EBC3", "F2EAC1", "F2E9BF", "F2E9BD", "F2E8BB", "F2E7B9", "F2E7B7", "F2E6B5", "F3E5B3", "F3E5B1", "F3E4AF", "F3E3AD", "F3E3AB", "F3E2A9", "F3E1A7", "F3E1A5", "F4E0A3", "F4DFA1", "F4DF9F", "F4DE9D", "F4DD9B", "F4DD99", "F4DC97", "F5DB95", "F5DB93", "F5DA91", "F5D98F", "F5D98D", "F5D88B", "F5D789", "F5D787", "F6D685", "F6D583", "F6D581", "F6D47F", "F6D37D", "F6D37B", "F6D279", "F7D177", "F7D175", "F7D073", "F7CF71", "F7CF6F", "F7CE6D", "F7CD6B", "F7CD69", "F8CC67", "F8CB65", "F8CB63", "F8CA61", "F8C95F", "F8C95D", "F8C85B", "F9C759", "F9C757", "F9C655", "F9C553", "F9C551", "F9C44F", "F9C34D", "F9C34B", "FAC249", "FAC147", "FAC145", "FAC043", "FABF41", "FABF3F", "FABE3D", "FBBD3B", "FBBD39", "FBBC37", "FBBB35", "FBBB33", "FBBA31", "FBB92F", "FBB92D", "FCB82B", "FCB729", "FCB727", "FCB625", "FCB523", "FCB521", "FCB41F", "FDB31D", "FDB31B", "FDB219", "FDB117", "FDB115", "FDB013", "FDAF11", "FDAF0F", "FEAE0D", "FEAD0B", "FEAD09", "FEAC07", "FEAB05", "FEAB03", "FEAA01", "FFAA00", "FCA800", "FAA700", "F8A600", "F6A400", "F4A300", "F2A200", "F0A000", "EE9F00", "EC9E00", "EA9C00", "E89B00", "E69A00", "E49800", "E29700", "E09600", "DE9400", "DC9300", "DA9200", "D89000", "D68F00", "D48E00", "D28C00", "D08B00", "CE8A00", "CC8800", "CA8700", "C88600", "C68400", "C48300", "C28200", "C08000", "BE7F00", "BC7E00", "BA7D00", "B87B00", "B67A00", "B47900", "B27700", "B07600", "AE7500", "AC7300", "AA7200", "A87100", "A66F00", "A46E00", "A26D00", "A06B00", "9E6A00", "9C6900", "9A6700", "986600", "966500", "946300", "926200", "906100", "8E5F00", "8C5E00", "8A5D00", "885B00", "865A00", "845900", "825700", "805600", "7E5500", "7C5400", "7A5200", "785100", "765000", "744E00", "724D00", "704C00", "6E4A00", "6C4900", "6A4800", "684600", "664500", "644400", "624200", "604100", "5E4000", "5C3E00", "5A3D00", "583C00", "563A00", "543900", "523800", "503600", "4E3500", "4C3400", "4A3200", "483100", "463000", "442E00", "422D00", "402C00", "3E2B00", "3C2900", "3A2800", "382700", "362500", "342400", "322300", "302100", "2E2000", "2C1F00", "2A1D00", "281C00", "261B00", "241900", "221800", "201700", "1E1500", "1C1400", "1A1300", "181100", "161000", "140F00", "120D00", "100C00", "0E0B00", "0C0900", "0A0800", "080700", "060500", "040400", "020300", "000200" };

/* Coloring method from https://stackoverflow.com/a/25816111 */
static Color getColor(const int i, double r, double c) {
	double size = sqrt(r * r + c * c);
	double ONE_OVER_LOG2 = 1.442695040889;
	double smoothed = log(log(size) * ONE_OVER_LOG2) * ONE_OVER_LOG2;
	int colorI = (int)(sqrt(i + 1 - smoothed) * 256 ) % 512;
	int cr, cg, cb;
	sscanf_s(hex[colorI].c_str(), "%02x%02x%02x", &cr, &cg, &cb);
	return Color{ cr,cg,cb };
}

int main(void)
{
	/* Spiral */
	//double dx = 0.2549870375144766;
	//double dy = -0.0005679790528465;
	//double zoom = 10E-14;
	
	/* Deep Swirl */
	//double dx = -1.315180982097868;
	//double dy = 0.073481649996795;

	/* Spider */
	double dx = -0.16070135;
	double dy = 1.0375665;
	double zoom = 10E-7;

	MandelbrotRenderer* r1{ new MandelbrotRenderer{ 500, 10000, zoom, dx,dy } };
	r1->enableZoom = false;
	r1->run();
	delete r1;

	

	std::cin.get();
}


static void drawMandelbrot(GLFWwindow* window, const unsigned int size, const double dx, const double dy, const double zoom, const unsigned int maxIterations) {

	std::cout << "Drawing..." << std::endl;
	std::cout << "Zoom:" << zoom << std::endl;
	std::cout << "Dx:" << dx << std::endl;
	std::cout << "Dy:" << dy << std::endl;

	char* data = new char[size*size * 3];

	for (unsigned int x = 0; x < size; x++) {
		for (unsigned int y = 0; y < size; y++) {

			unsigned int i = (x + y * size) * 3;

			ManVal v = getMandelbrotValue(size, x, y, dx, dy, zoom, maxIterations);
			Color c{ getColor(v.i,v.r,v.c) };

			data[i + 0] = c.r;
			data[i + 1] = c.g;
			data[i + 2] = c.b;

		}

		// Push pixels every 10% of image
		//if (x % (size / 10) == 0)
		{
			glDrawPixels(size, size, GL_RGB, GL_UNSIGNED_BYTE, data);
			glfwSwapBuffers(window);
		}


	}

	std::cout << "Calculation done!" << std::endl;
	glDrawPixels(size, size, GL_RGB, GL_UNSIGNED_BYTE, data);
	glfwSwapBuffers(window);

	const int dimx = size, dimy = size;
	int i, j;

	FILE *fp;
	errno_t err;

	if ((err = fopen_s(&fp, "first.ppm", "wb")) != 0) {
		std::cerr << "Error opening file!" << std::endl;
	}
	else {
		// File was opened, filepoint can be used to read the stream.
	}

	(void)fprintf(fp, "P6\n%d %d\n255\n", dimx, dimy);
	fwrite(data, sizeof(char)*size*size*3, 1, fp);
	(void)fclose(fp);

	delete[] data;

}

static ManVal getMandelbrotValue(const unsigned int size, const unsigned int x, const unsigned int y, const double dx, const double dy, const double zoom, const unsigned int maxIterations) {

	// Map pixel position between minR and maxR
	double a = map(x, 0, size, -zoom, zoom);
	double b = map(y, 0, size, -zoom, zoom);
	a += dx;
	b += dy;
	int n = 0;
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

	return ManVal{a,b,n};

}