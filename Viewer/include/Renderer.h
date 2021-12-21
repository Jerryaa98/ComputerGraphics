#pragma once
#include "Scene.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

class Renderer
{
public:
	Renderer(int viewportWidth, int viewportHeight);
	virtual ~Renderer();
	void Render(const Scene& scene, std::shared_ptr<MeshModel> cameraModel);
	void SwapBuffers();
	void ClearColorBuffer(const glm::vec3& color);
	int GetViewportWidth() const;
	int GetViewportHeight() const;

	void setViewportWidth(const int width);
	void setViewportHeight(const int height);

	// this is used for sanity check
	void DrawCircle(const glm::ivec3& center, const float radius, const int stepSize);

	void CreateBuffers(int w, int h);

	float maxZ = -1.0f * FLT_MAX;
	float minZ = FLT_MAX;

private:
	void PutPixel(const int i, const int j, const glm::vec3& color, float depth);
	void DrawLine(const glm::ivec3& p1, const glm::ivec3& p2, const glm::vec3& color);

	bool pointInTriangle(const glm::ivec2& curr_point, const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3);

	float triangleArea(glm::vec3 v, glm::vec3 u, glm::vec2 w);


	// DrawLine helpers
	void DrawLineHigh(const glm::ivec3& p1, const glm::ivec3& p2, const glm::vec3& color);
	void DrawLineLow(const glm::ivec3& p1, const glm::ivec3& p2, const glm::vec3& color);
	
	void DrawTriangle(glm::vec3& p1, glm::vec3& p2, glm::vec3& p3, glm::vec3& color, MeshModel& model);


	void DrawCat(const int scale);

	void CreateOpenglBuffer();
	void InitOpenglRendering();

	float* color_buffer;
	float* z_buffer;
	int viewport_width;
	int viewport_height;
	GLuint gl_screen_tex;
	GLuint gl_screen_vtc;
};
