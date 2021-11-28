#pragma once
#include "Scene.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

class Renderer
{
public:
	Renderer(int viewportWidth, int viewportHeight);
	virtual ~Renderer();
	void Render(const Scene& scene);
	void SwapBuffers();
	void ClearColorBuffer(const glm::vec3& color);
	int GetViewportWidth() const;
	int GetViewportHeight() const;

	void setViewportWidth(const int width);
	void setViewportHeight(const int height);

	// this is used for sanity check
	void DrawCircle(const glm::ivec2& center, const float radius, const int stepSize);

	void CreateBuffers(int w, int h);
private:
	void PutPixel(const int i, const int j, const glm::vec3& color);
	void DrawLine(const glm::ivec2& p1, const glm::ivec2& p2, const glm::vec3& color);

	// DrawLine helpers
	void DrawLineHigh(const glm::ivec2& p1, const glm::ivec2& p2, const glm::vec3& color);
	void DrawLineLow(const glm::ivec2& p1, const glm::ivec2& p2, const glm::vec3& color);


	void DrawCat(const int scale);

	void CreateOpenglBuffer();
	void InitOpenglRendering();

	float* color_buffer;
	int viewport_width;
	int viewport_height;
	GLuint gl_screen_tex;
	GLuint gl_screen_vtc;
};
