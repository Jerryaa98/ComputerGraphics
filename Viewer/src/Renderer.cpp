#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include "Renderer.h"
#include "InitShader.h"
#include "Utils.h"
#include <glm/glm.hpp>
#include <glm\gtx\string_cast.hpp>

#define INDEX(width,x,y,c) ((x)+(y)*(width))*3+(c)
#define Z_INDEX(width,x,y) ((x)+(y)*(width))

# define CAT_SCALE 40

Renderer::Renderer(int viewport_width, int viewport_height) :
	viewport_width(viewport_width),
	viewport_height(viewport_height)
{
	InitOpenglRendering();
	CreateBuffers(viewport_width, viewport_height);
}

Renderer::~Renderer()
{
	delete[] color_buffer;
}

void Renderer::PutPixel(int i, int j, const glm::vec3& color, float depth)
{
	if (i < 0) return; if (i >= viewport_width) return;
	if (j < 0) return; if (j >= viewport_height) return;

	if (z_buffer[Z_INDEX(viewport_width, i, j)] > depth)
		return;

	color_buffer[INDEX(viewport_width, i, j, 0)] = color.x;
	color_buffer[INDEX(viewport_width, i, j, 1)] = color.y;
	color_buffer[INDEX(viewport_width, i, j, 2)] = color.z;
	z_buffer[Z_INDEX(viewport_width, i, j)] = depth;
}

void Renderer::DrawCircle(const glm::ivec3& center, const float radius, const int stepSize) {
	glm::vec3 color = glm::vec3(1, 1, 1);

	for (int i = 0; i < stepSize; i++) {
		float x = center.x + (radius * (sin(((2 * M_PI * i) / stepSize))));
		float y = center.y + (radius * (cos(((2 * M_PI * i) / stepSize))));
		glm::vec3 endPoint = glm::vec3(x, y, 0);
		DrawLine(center, endPoint, color);
	}
}


void Renderer::DrawLineHigh(const glm::ivec3& p1, const glm::ivec3& p2, const glm::vec3& color) {
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	int xi = 1;
	float d, x;

	float z = (p1.z + p2.z) / 2;

	if (dx < 0) {
		xi = -1;
		dx = ((-1) * dx);
	}

	d = (2 * dx) - dy;
	x = p1.x;
	for (int y = p1.y; y < p2.y; y++) {
		PutPixel(x, y, color, z);
		if (d > 0) {
			x += xi;
			d += (2 * (dx - dy));
		}
		else {
			d += (2 * dx);
		}
	}
}

void Renderer::DrawLineLow(const glm::ivec3& p1, const glm::ivec3& p2, const glm::vec3& color) {
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	int yi = 1;
	float d, y;

	float z = (p1.z + p2.z) / 2;

	if (dy < 0) {
		yi = -1;
		dy = ((-1) * dy);
	}

	d = (2 * dy) - dx;
	y = p1.y;
	for (int x = p1.x; x < p2.x; x++) {
		PutPixel(x, y, color, z);
		if (d > 0) {
			y += yi;
			d += (2 * (dy - dx));
		}
		else {
			d += (2 * dy);
		}
	}
}

void Renderer::DrawLine(const glm::ivec3& p1, const glm::ivec3& p2, const glm::vec3& color)
{
	// bresenham algorithm https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

	float dx = abs(p1.x - p2.x);
	float dy = abs(p1.y - p2.y);

	if (dy < dx) {
		if (p1.x > p2.x) {
			DrawLineLow(p2, p1, color);
			return;
		}
		else {
			DrawLineLow(p1, p2, color);
			return;
		}
	}
	else {
		if (p1.y > p2.y) {
			DrawLineHigh(p2, p1, color);
			return;
		}
		else {
			DrawLineHigh(p1, p2, color);
			return;
		}
	}


}

void Renderer::CreateBuffers(int w, int h)
{
	CreateOpenglBuffer(); //Do not remove this line.
	color_buffer = new float[3 * w * h];
	z_buffer = new float[w * h];
	ClearColorBuffer(glm::vec3(0.0f, 0.0f, 0.0f));
}

//##############################
//##OpenGL stuff. Don't touch.##
//##############################

// Basic tutorial on how opengl works:
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
// don't linger here for now, we will have a few tutorials about opengl later.
void Renderer::InitOpenglRendering()
{
	// Creates a unique identifier for an opengl texture.
	glGenTextures(1, &gl_screen_tex);

	// Same for vertex array object (VAO). VAO is a set of buffers that describe a renderable object.
	glGenVertexArrays(1, &gl_screen_vtc);

	GLuint buffer;

	// Makes this VAO the current one.
	glBindVertexArray(gl_screen_vtc);

	// Creates a unique identifier for a buffer.
	glGenBuffers(1, &buffer);

	// (-1, 1)____(1, 1)
	//	     |\  |
	//	     | \ | <--- The exture is drawn over two triangles that stretch over the screen.
	//	     |__\|
	// (-1,-1)    (1,-1)
	const GLfloat vtc[] = {
		-1, -1,
		 1, -1,
		-1,  1,
		-1,  1,
		 1, -1,
		 1,  1
	};

	const GLfloat tex[] = {
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1 };

	// Makes this buffer the current one.
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	// This is the opengl way for doing malloc on the gpu. 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc) + sizeof(tex), NULL, GL_STATIC_DRAW);

	// memcopy vtc to buffer[0,sizeof(vtc)-1]
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);

	// memcopy tex to buffer[sizeof(vtc),sizeof(vtc)+sizeof(tex)]
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	// Loads and compiles a sheder.
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");

	// Make this program the current one.
	glUseProgram(program);

	// Tells the shader where to look for the vertex position data, and the data dimensions.
	GLint  vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Same for texture coordinates data.
	GLint  vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(vtc));

	//glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );

	// Tells the shader to use GL_TEXTURE0 as the texture id.
	glUniform1i(glGetUniformLocation(program, "texture"), 0);
}

void Renderer::CreateOpenglBuffer()
{
	// Makes GL_TEXTURE0 the current active texture unit
	glActiveTexture(GL_TEXTURE0);

	// Makes glScreenTex (which was allocated earlier) the current texture.
	glBindTexture(GL_TEXTURE_2D, gl_screen_tex);

	// malloc for a texture on the gpu.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, viewport_width, viewport_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, viewport_width, viewport_height);
}

void Renderer::SwapBuffers()
{
	// Makes GL_TEXTURE0 the current active texture unit
	glActiveTexture(GL_TEXTURE0);

	// Makes glScreenTex (which was allocated earlier) the current texture.
	glBindTexture(GL_TEXTURE_2D, gl_screen_tex);

	// memcopy's colorBuffer into the gpu.
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, viewport_width, viewport_height, GL_RGB, GL_FLOAT, color_buffer);

	// Tells opengl to use mipmapping
	glGenerateMipmap(GL_TEXTURE_2D);

	// Make glScreenVtc current VAO
	glBindVertexArray(gl_screen_vtc);

	// Finally renders the data.
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::ClearColorBuffer(const glm::vec3& color)
{
	for (int i = 0; i < viewport_width; i++)
	{
		for (int j = 0; j < viewport_height; j++)
		{
			//PutPixel(i, j, color, FLT_MAX);
			color_buffer[INDEX(viewport_width, i, j, 0)] = color.x;
			color_buffer[INDEX(viewport_width, i, j, 1)] = color.y;
			color_buffer[INDEX(viewport_width, i, j, 2)] = color.z;
			z_buffer[Z_INDEX(viewport_width, i, j)] = -1.0f * FLT_MAX;
		}
	}
}

bool Renderer::pointInTriangle(const glm::ivec2& point, const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3) {
	float val1 = (p1.x - p3.x) * (point.y - p3.y) - (p1.y - p3.y) * (point.x - p3.x);
	float val2 = (p2.x - p1.x) * (point.y - p1.y) - (p2.y - p1.y) * (point.x - p1.x);

	if ((val1 < 0) != (val2 < 0) && val1 != 0 && val2 != 0)
		return false;

	float val3 = (p3.x - p2.x) * (point.y - p2.y) - (p3.y - p2.y) * (point.x - p2.x);

	if (val3 == 0 || ((val3 < 0) == (val1 + val2 <= 0)))
		return true;

	return false;
}

float Renderer::triangleArea(glm::vec3 p1, glm::vec3 p2, glm::vec2 w) {
	float vu, uw, vw, dx, dy, s;
	dx = p1.x - w.x;
	dy = p1.y - w.y;
	vw = sqrt(dx * dx + dy * dy);
	dx = p2.x - w.x;
	dy = p2.y - w.y;
	uw = sqrt(dx * dx + dy * dy);
	dx = p1.x - p2.x;
	dy = p1.y - p2.y;
	vu = sqrt(dx * dx + dy * dy);
	s = (vu + uw + vw) / 2;
	return  sqrt(s * (s - vu) * (s - uw) * (s - vw));
}

//glm::vec3 Renderer::color( ,const Scene& scene1,MeshModel& model) {
//	
//	for (int i = 0; i < scene.GetLightCount(); i++) {
//		Light& light = scene.GetLight(i);
//		glm::vec3 finalColor = glm::vec3(0.0f, 0.0f, 0.0f);
//
//		// Ambient
//		glm::vec3 Ia = interpolation * light.ambientColor* model.ambientColor;
//
//		// Diffuse
//		glm::vec3 lightVector = glm::vec3(x, y, z) - light.updatedLocation;
//		glm::vec3 Id = interpolation * light.diffuseColor * glm::dot(normal, lightVector) * model.diffuseColor;
//
//
//		// Specular
//		glm::vec3 I = glm::normalize(light.updatedLocation - glm::vec3(x, y, z));
//		glm::vec3 reflection = I - 2 * glm::dot(I, glm::normalize(normal)) * glm::normalize(normal);
//		float* arr = scene.GetActiveCamera().eye;
//		glm::vec3 roro = glm::vec3(arr[0], arr[1], arr[2]);
//		glm::vec3 Is = interpolation * light.specularColor * (float)pow(max(0.0f, glm::dot((roro - glm::vec3(x, y, z)), reflection)), light.alpha)* model.specularColor;
//		//cout << glm::to_string(Is) << endl;
//		PutPixel(x, y, Ia + Id + Is, z);
//	}
//	
//}

void Renderer::DrawTriangle(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, MeshModel& model, int faceIndex, Scene& scene) {
	if (model.trianglesOutlines) {
		DrawLine(p1, p2, glm::vec3(1, 1, 1));
		DrawLine(p2, p3, glm::vec3(1, 1, 1));
		DrawLine(p1, p3, glm::vec3(1, 1, 1));
	}
	
	float xMin = min(min(p1.x, p2.x), p3.x);
	float yMin = min(min(p1.y, p2.y), p3.y);

	float xMax = max(max(p1.x, p2.x), p3.x);
	float yMax = max(max(p1.y, p2.y), p3.y);

	if (model.trianglesBoundingBoxes) {
		glm::vec3 randomColor = glm::vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
		DrawLine(glm::vec3(xMin, yMin, 100), glm::vec3(xMin, yMax, 100), randomColor);
		DrawLine(glm::vec3(xMax, yMin, 100), glm::vec3(xMax, yMax, 100), randomColor);
		DrawLine(glm::vec3(xMin, yMax, 100), glm::vec3(xMax, yMax, 100), randomColor);
		DrawLine(glm::vec3(xMax, yMin, 100), glm::vec3(xMin, yMin, 100), randomColor);
	}

	if (model.coloredTriangles) {
		float a1 = 0;
		float a2 = 0;
		float a3 = 0;
		float A = 0;
		float z = 0;
		
		glm::vec3 U = p2 - p1;
		glm::vec3 V = p3 - p1;

		float x = (U.y * V.z) - (U.z * V.y);
		float y = (U.z * V.x) - (U.x * V.z);
		float z1 = (U.x * V.y) - (U.y * V.x);

		glm::vec3 normal = glm::normalize(glm::vec3(x, y, z1));

		for (int y = yMax; y >= yMin; y--) {
			for (int x = xMin; x <= xMax; x++) {
				glm::ivec2 point = glm::ivec2(x, y);
				if (pointInTriangle(point, p1, p2, p3)) {
					a1 = triangleArea(p1, p2, point);
					a2 = triangleArea(p2, p3, point);
					a3 = triangleArea(p1, p3, point);

					A = a1 + a2 + a3;
					z = ((a1 * p3.z) / A) + ((a2 * p1.z) / A) + ((a3 * p2.z) / A);
					//float interpolation = 1.0f - ((this->maxZ - z) / (this->maxZ - this->minZ));
					float interpolation = 1.0f;
					
					for (int i = 0; i < scene.GetLightCount(); i++) {
						Light& light = scene.GetLight(i);
						

						// Ambient
						glm::vec3 Ia = light.ambientColor * model.ambientColor;
						//cout << glm::to_string(Ia) << endl;


						// Diffuse
						glm::vec3 lightVector = glm::normalize(light.updatedLocation - glm::vec3(x, y, z));
						glm::vec3 Id = 1000.0f * light.diffuseColor * model.diffuseColor * max((float)glm::dot(normal, lightVector), 0.0f);
						//cout << glm::to_string(Id) << endl;


						// Specular
						float* arr = scene.GetActiveCamera().eye;
						glm::vec3 R = glm::normalize((2 * glm::dot(lightVector, normal) * normal) - lightVector);
						glm::vec3 cameraLocation = glm::vec3(arr[0], arr[1], arr[2]);
						glm::vec3 V = glm::normalize(cameraLocation - glm::vec3(x, y, z));
						glm::vec3 Is = light.specularColor * model.specularColor * pow(max((float)glm::dot(R, V), 0.0f), light.alpha);
						
						glm::vec3 finalColor = 0.3f * (Ia + Id + Is);
						//finalColor /= max(max(finalColor.x, finalColor.y), finalColor.z);
						PutPixel(x, y, finalColor, z);
					}
				}
			}
		}

	}
}

void Renderer::Render(const Scene& scene, std::shared_ptr<MeshModel> cameraModel)
{

	Scene scene1 = scene;
	// TODO: Replace this code with real scene rendering code
	int half_width = viewport_width / 2;
	int half_height = viewport_height / 2;

	// draw circle
	//DrawCircle(glm::ivec2(half_width, half_height), 150, 36);

	// draw cat
	//DrawCat(CAT_SCALE);
	scene1.GetActiveCamera().half_height = half_height;
	scene1.GetActiveCamera().half_width = half_width;

	int numOfModels = scene.GetModelCount();
	int numOfCamera = scene.GetCameraCount();
	int activeCamera = scene.GetActiveCameraIndex();
	glm::mat4x4 cameraTransform = scene1.GetActiveCamera().GetTransform();

	glm::vec3 color = glm::vec3(255, 255, 255);

	maxZ = -1.0f * FLT_MAX;
	minZ = FLT_MAX;

	for (int i = 0; i < numOfModels; i++) {
		MeshModel& model = scene.GetModel(i);
		this->maxZ = max(this->maxZ, model.maxZ);
		this->minZ = min(this->minZ, model.minZ);
	}

	for (int i = 0; i < scene.GetLightCount(); i++) {
		scene1.GetLight(i).GetTransform(cameraTransform);
	}

	for (int i = 0; i < numOfModels; i++) {
		MeshModel& model = scene.GetModel(i);

		glm::mat4x4 trans = glm::mat4x4(1.0f);
		trans[3][0] = 0;
		trans[3][1] = 0;
		trans[3][2] = 1;

		std::vector<glm::vec3> newVertices = model.Draw(cameraTransform);

		for (int j = 0; j < model.GetFacesCount(); j++) {
			Face face = model.GetFace(j);

			// VERTICES
			int v1Index = face.GetVertexIndex(0) - 1;
			int v2Index = face.GetVertexIndex(1) - 1;
			int v3Index = face.GetVertexIndex(2) - 1;

			glm::vec3 v1 = newVertices.at(v1Index);
			glm::vec3 v2 = newVertices.at(v2Index);
			glm::vec3 v3 = newVertices.at(v3Index);

			v1.x = (v1.x + 1) * half_width;
			v1.y = (v1.y + 1) * half_height;

			v2.x = (v2.x + 1) * half_width;
			v2.y = (v2.y + 1) * half_height;

			v3.x = (v3.x + 1) * half_width;
			v3.y = (v3.y + 1) * half_height;

			DrawTriangle(v1, v2, v3, model, j, scene1);
		}

		if (model.drawAxis) {
			std::vector<glm::vec3> points = model.transformedAxis;

			points.at(0).x = (points.at(0).x + 1) * half_width;
			points.at(0).y = (points.at(0).y + 1) * half_height;
			
			points.at(1).x = (points.at(1).x + 1) * half_width;
			points.at(1).y = (points.at(1).y + 1) * half_height;

			points.at(2).x = (points.at(2).x + 1) * half_width;
			points.at(2).y = (points.at(2).y + 1) * half_height;

			points.at(3).x = (points.at(3).x + 1) * half_width;
			points.at(3).y = (points.at(3).y + 1) * half_height;

			points.at(4).x = (points.at(4).x + 1) * half_width;
			points.at(4).y = (points.at(4).y + 1) * half_height;

			points.at(5).x = (points.at(5).x + 1) * half_width;
			points.at(5).y = (points.at(5).y + 1) * half_height;

			//X axis
			DrawLine(points.at(0), points.at(1), glm::vec3(255, 0, 0));
			//Y axis
			DrawLine(points.at(2), points.at(3), glm::vec3(0, 255, 0));
			//Z axis
			DrawLine(points.at(4), points.at(5), glm::vec3(0, 0, 255));
		}

		if (model.drawBoundingBox) {
			std::vector<glm::vec3> points = model.transformedBoundingBox;
	
			points.at(0).x = (points.at(0).x + 1) * half_width;
			points.at(0).y = (points.at(0).y + 1) * half_height;

			points.at(1).x = (points.at(1).x + 1) * half_width;
			points.at(1).y = (points.at(1).y + 1) * half_height;

			points.at(2).x = (points.at(2).x + 1) * half_width;
			points.at(2).y = (points.at(2).y + 1) * half_height;

			points.at(3).x = (points.at(3).x + 1) * half_width;
			points.at(3).y = (points.at(3).y + 1) * half_height;

			points.at(4).x = (points.at(4).x + 1) * half_width;
			points.at(4).y = (points.at(4).y + 1) * half_height;

			points.at(5).x = (points.at(5).x + 1) * half_width;
			points.at(5).y = (points.at(5).y + 1) * half_height;

			points.at(6).x = (points.at(6).x + 1) * half_width;
			points.at(6).y = (points.at(6).y + 1) * half_height;

			points.at(7).x = (points.at(7).x + 1) * half_width;
			points.at(7).y = (points.at(7).y + 1) * half_height;

			DrawLine(points.at(0), points.at(1), glm::vec3(255, 0, 0));
			DrawLine(points.at(0), points.at(3), glm::vec3(255, 0, 0));
			DrawLine(points.at(0), points.at(4), glm::vec3(255, 0, 0));
			DrawLine(points.at(1), points.at(2), glm::vec3(255, 0, 0));
			DrawLine(points.at(1), points.at(5), glm::vec3(255, 0, 0));
			DrawLine(points.at(2), points.at(3), glm::vec3(255, 0, 0));
			DrawLine(points.at(2), points.at(6), glm::vec3(255, 0, 0));
			DrawLine(points.at(3), points.at(7), glm::vec3(255, 0, 0));
			DrawLine(points.at(4), points.at(5), glm::vec3(255, 0, 0));
			DrawLine(points.at(4), points.at(7), glm::vec3(255, 0, 0));
			DrawLine(points.at(5), points.at(6), glm::vec3(255, 0, 0));
			DrawLine(points.at(6), points.at(7), glm::vec3(255, 0, 0));
			// 12 14 15 23 26 34 37 48 56 58 67 78
		}

		if (model.drawVertexNormals) {
			for each (std::vector<glm::vec3> pair in model.transformedVertexNormals) {
				pair.at(0).x = (pair.at(0).x + 1) * half_width;
				pair.at(0).y = (pair.at(0).y + 1) * half_height;

				pair.at(1).x = (pair.at(1).x + 1) * half_width;
				pair.at(1).y = (pair.at(1).y + 1) * half_height;

				DrawLine(pair.at(0), pair.at(1), glm::vec3(0, 0, 255));
			}
		}

		if (model.drawFaceNormals) {
			for each (std::vector<glm::vec3> pair in model.transformedFaceNormals) {
				pair.at(0).x = (pair.at(0).x + 1) * half_width;
				pair.at(0).y = (pair.at(0).y + 1) * half_height;

				pair.at(1).x = (pair.at(1).x + 1) * half_width;
				pair.at(1).y = (pair.at(1).y + 1) * half_height;

				DrawLine(pair.at(0), pair.at(1), glm::vec3(0, 255, 0));
			}
		}


	}

	if (scene.drawWorldAxis) {
		std::vector<glm::vec3> axis;
		// X axis
		glm::vec4 x1 = cameraTransform * glm::vec4(0, 0, 0, 1);
		glm::vec4 x2 = cameraTransform * glm::vec4(half_height / 2, 0, 0, 1);

		glm::vec3 x1Transformed = glm::vec3(x1 / x1.w);
		x1Transformed.x = (x1Transformed.x + 1) * half_width;
		x1Transformed.y = (x1Transformed.y + 1) * half_height;
		glm::vec3 x2Transformed = glm::vec3(x2 / x2.w);
		x2Transformed.x = (x2Transformed.x + 1) * half_width;
		x2Transformed.y = (x2Transformed.y + 1) * half_height;

		axis.push_back(x1Transformed);
		axis.push_back(x2Transformed);

	

		// Y axis
		glm::vec4 y1 = cameraTransform * glm::vec4(0, 0, 0, 1);
		glm::vec4 y2 = cameraTransform * glm::vec4(0, half_width / 2, 0, 1);
		glm::vec3 y1Transformed = glm::vec3(y1 / y1.w);
		y1Transformed.x = (y1Transformed.x + 1) * half_width;
		y1Transformed.y = (y1Transformed.y + 1) * half_height;
		glm::vec3 y2Transformed = glm::vec3(y2 / y2.w);
		y2Transformed.x = (y2Transformed.x + 1) * half_width;
		y2Transformed.y = (y2Transformed.y + 1) * half_height;

		axis.push_back(y1Transformed);
		axis.push_back(y2Transformed);

		// Z axis
		glm::vec4 z1 = cameraTransform * glm::vec4(0, 0, 0, 1);
		glm::vec4 z2 = cameraTransform * glm::vec4(0, 0, half_width / 2, 1);
		glm::vec3 z1Transformed = glm::vec3(z1 / z1.w);
		z1Transformed.x = (z1Transformed.x + 1) * half_width;
		z1Transformed.y = (z1Transformed.y + 1) * half_height;
		glm::vec3 z2Transformed = glm::vec3(z2 / z2.w);
		z2Transformed.x = (z2Transformed.x + 1) * half_width;
		z2Transformed.y = (z2Transformed.y + 1) * half_height;

		axis.push_back(z1Transformed);
		axis.push_back(z2Transformed);

		//axis.at(0).x += half_width;
		//axis.at(0).y += half_height;
		//axis.at(1).x += half_width;
		//axis.at(1).y += half_height;
		//axis.at(2).x += half_width;
		//axis.at(2).y += half_height;
		//axis.at(3).x += half_width;
		//axis.at(3).y += half_height;
		//axis.at(4).x += half_width;
		//axis.at(4).y += half_height;
		//axis.at(5).x += half_width;
		//axis.at(5).y += half_height;

		//X axis
		DrawLine(axis.at(0), axis.at(1), glm::vec3(255, 0, 0));
		//Y axis
		DrawLine(axis.at(2), axis.at(3), glm::vec3(0, 255, 0));
		//Z axis
		DrawLine(axis.at(4), axis.at(5), glm::vec3(0, 0, 255));
	}

	if (numOfCamera > 1) {
		for (int i = 0; i < numOfCamera; i++) {
			if (i == activeCamera)
				continue;
		
			(*cameraModel).localTranslateArray[0] = scene1.GetCamera(i).localTranslateArray[0] + scene1.GetCamera(i).eye[0];
			(*cameraModel).localTranslateArray[1] = scene1.GetCamera(i).localTranslateArray[1] + scene1.GetCamera(i).eye[1];
			(*cameraModel).localTranslateArray[2] = scene1.GetCamera(i).localTranslateArray[2] + scene1.GetCamera(i).eye[2];
			
			(*cameraModel).localRotateArray[0] = scene1.GetCamera(i).localRotateArray[0];
			(*cameraModel).localRotateArray[1] = scene1.GetCamera(i).localRotateArray[1];
			(*cameraModel).localRotateArray[2] = scene1.GetCamera(i).localRotateArray[2];
			
			(*cameraModel).worldTranslateArray[0] = scene1.GetCamera(i).worldTranslateArray[0];
			(*cameraModel).worldTranslateArray[1] = scene1.GetCamera(i).worldTranslateArray[1];
			(*cameraModel).worldTranslateArray[2] = scene1.GetCamera(i).worldTranslateArray[2];
			
			(*cameraModel).worldRotateArray[0] = scene1.GetCamera(i).worldRotateArray[0];
			(*cameraModel).worldRotateArray[1] = scene1.GetCamera(i).worldRotateArray[1];
			(*cameraModel).worldRotateArray[2] = scene1.GetCamera(i).worldRotateArray[2];

			std::vector<glm::vec3> newVertices = (*cameraModel).Draw(cameraTransform);

			for (int j = 0; j < (*cameraModel).GetFacesCount(); j++) {
				Face face = (*cameraModel).GetFace(j);

				// VERTICES
				int v1Index = face.GetVertexIndex(0) - 1;
				int v2Index = face.GetVertexIndex(1) - 1;
				int v3Index = face.GetVertexIndex(2) - 1;

				glm::vec3 v1 = newVertices.at(v1Index);
				glm::vec3 v2 = newVertices.at(v2Index);
				glm::vec3 v3 = newVertices.at(v3Index);

				v1.x = (v1.x + 1) * half_width;
				v1.y = (v1.y + 1) * half_height;

				v2.x = (v2.x + 1) * half_width;
				v2.y = (v2.y + 1) * half_height;

				v3.x = (v3.x + 1) * half_width;
				v3.y = (v3.y + 1) * half_height;

				DrawLine(v1, v2, color);
				DrawLine(v2, v3, color);
				DrawLine(v1, v3, color);
			}
		}

	}
}

int Renderer::GetViewportWidth() const
{
	return viewport_width;
}

int Renderer::GetViewportHeight() const
{
	return viewport_height;
}

void Renderer::setViewportWidth(const int width) {
	this->viewport_width = width;
}
void Renderer::setViewportHeight(const int height) {
	this->viewport_height = height;
}