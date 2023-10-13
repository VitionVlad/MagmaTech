#include <iostream>

#include "Render.hpp"

class Engine {
private:
	bool begun = false;
#if defined(__ANDROID__)
	ANativeWindow* lwindow;
#endif
public:
	Render ren;
#if defined(__ANDROID__)
	void init(std::string appname, ANativeWindow* window) {
		lwindow = window;
		ren.init(appname, window);
	}
#else
	void init(std::string appname) {
		ren.init(appname);
	}
#endif
	void beginShadowPass() {
		if (!begun) {
#if defined(__ANDROID__)
			ren.beginRender(lwindow);
#else
			ren.beginRender();
#endif
			begun = true;
		}
		ren.beginShadowPass();
	}
	void beginMainPass() {
		if (!begun) {
#if defined(__ANDROID__)
			ren.beginRender(lwindow);
#else
			ren.beginRender();
#endif
			begun = true;
		}
		ren.beginMainPass();
	}
	void endRender() {
		ren.endRender();
		begun = false;
	}
	void terminate() {
		ren.terminate();
	}
};

class Object {
private:
	Loader assets;
	Loader cube;
public:
	Mesh mesh;
	void create(Engine& eng, std::string vertshader, std::string fragshader, glm::vec3* vertexes, glm::vec2* uv, glm::vec3* normals, int size, unsigned char* pixels, glm::ivec2 TexResolution, int imagecount, unsigned char* cpixels, glm::ivec2 cubeResolution, int cubecount) {
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, vertexes, uv, normals, size, pixels, TexResolution, imagecount, cpixels, cubeResolution, cubecount);
	}
	void create(Engine& eng, std::string vertshader, std::string fragshader, std::string modelpath, std::string* imagespath, int imagecount, std::string* cubespath, int cubescount) {
		assets.loadobj(eng.ren.pathprefix + modelpath);
		for (int i = 0; i != imagecount; i++) {
			assets.loadppm(eng.ren.pathprefix + imagespath[i]);
		}
		for (int i = 0; i != cubescount * 6; i++) {
			cube.loadppm(eng.ren.pathprefix + cubespath[i]);
		}
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), assets.pixels.data(), assets.textureResolution, imagecount, cube.pixels.data(), cube.textureResolution, cubescount);
	}
	void Draw(Engine& eng) {
		mesh.Draw(eng.ren);
	}
};