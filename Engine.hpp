#include <iostream>

#include "Render.hpp"

#include "Audio.hpp"

class Engine {
private:
	ma_result result;
	bool begun = false;
#if defined(__ANDROID__)
	ANativeWindow* lwindow;
#endif
public:
	ma_engine aud;
	Render ren;
	float volume = 1.0f;
#if defined(__ANDROID__)
	void init(std::string appname, ANativeWindow* window) {
		lwindow = window;
		ren.init(appname, window);
		result = ma_engine_init(NULL, &aud);
		if (result != MA_SUCCESS) {
			exit(result);
		}
	}
#else
	void init(std::string appname) {
		ren.init(appname);
		result = ma_engine_init(NULL, &aud);
		if (result != MA_SUCCESS) {
			std::cout << "error: failed to init miniaudio" << std::endl;
			exit(-1);
		}
		std::cout << "log: engine inited with success" << std::endl;
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
	Loader assets{};
	Loader cube{};
	bool withaudio = false;
public:
	Mesh mesh;
	AudioSource audio;
	glm::vec3 pos = glm::vec3(0, 0, 0);
	glm::vec3 rot = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	void create(Engine& eng, std::string vertshader, std::string fragshader, glm::vec3* vertexes, glm::vec2* uv, glm::vec3* normals, int size, unsigned char* pixels, glm::ivec2 TexResolution, int imagecount, unsigned char* cpixels, glm::ivec2 cubeResolution, int cubecount) {
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, vertexes, uv, normals, size, pixels, TexResolution, imagecount, cpixels, cubeResolution, cubecount);
	}
	void create(Engine& eng, std::string vertshader, std::string fragshader, std::string modelpath, std::string* imagespath, int imagecount, std::string* cubespath, int cubescount) {
		assets.loadobj(eng.ren.pathprefix + modelpath);
		std::cout << "log: model loaded" << std::endl;
		for (int i = 0; i != imagecount; i++) {
			assets.loadppm(eng.ren.pathprefix + imagespath[i]);
		}
		std::cout << "log: main textures loaded" << std::endl;
		for (int i = 0; i != cubescount * 6; i++) {
			cube.loadppm(eng.ren.pathprefix + cubespath[i]);
		}
		std::cout << "log: cubemaps loaded" << std::endl;
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), assets.pixels.data(), assets.textureResolution, imagecount, cube.pixels.data(), cube.textureResolution, cubescount);
		std::cout << "log: object created" << std::endl;
	}
	void createNoCube(Engine& eng, std::string vertshader, std::string fragshader, std::string modelpath, std::string* imagespath, int imagecount) {
		assets.loadobj(eng.ren.pathprefix + modelpath);
		std::cout << "log: model loaded" << std::endl;
		for (int i = 0; i != imagecount; i++) {
			assets.loadppm(eng.ren.pathprefix + imagespath[i]);
		}
		std::cout << "log: main textures loaded" << std::endl;
		unsigned char data[24];
		for (int i = 0; i != 24; i++) {
			data[i] = 255;
		}
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), assets.pixels.data(), assets.textureResolution, imagecount, data, glm::ivec2(1, 1), 1);
		std::cout << "log: object created" << std::endl;
	}
	void createNoTex(Engine& eng, std::string vertshader, std::string fragshader, std::string modelpath, std::string* cubespath, int cubescount) {
		assets.loadobj(eng.ren.pathprefix + modelpath);
		std::cout << "log: model loaded" << std::endl;
		unsigned char data[4] = { 255, 255, 255, 255 };
		for (int i = 0; i != cubescount * 6; i++) {
			cube.loadppm(eng.ren.pathprefix + cubespath[i]);
		}
		std::cout << "log: cubemaps loaded" << std::endl;
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), data, glm::ivec2(1, 1), 1, cube.pixels.data(), cube.textureResolution, cubescount);
		std::cout << "log: object created" << std::endl;
	}
	//with audio
	void create(Engine& eng, std::string vertshader, std::string fragshader, glm::vec3* vertexes, glm::vec2* uv, glm::vec3* normals, int size, unsigned char* pixels, glm::ivec2 TexResolution, int imagecount, unsigned char* cpixels, glm::ivec2 cubeResolution, int cubecount, std::string audiopath, float maxdist) {
		audio.maxdist = maxdist;
		audio.create(eng.aud, eng.ren.pathprefix + audiopath);
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, vertexes, uv, normals, size, pixels, TexResolution, imagecount, cpixels, cubeResolution, cubecount);
		withaudio = true;
	}
	void create(Engine& eng, std::string vertshader, std::string fragshader, std::string modelpath, std::string* imagespath, int imagecount, std::string* cubespath, int cubescount, std::string audiopath, float maxdist) {
		audio.maxdist = maxdist;
		audio.create(eng.aud, eng.ren.pathprefix + audiopath);
		assets.loadobj(eng.ren.pathprefix + modelpath);
		std::cout << "log: model loaded" << std::endl;
		for (int i = 0; i != imagecount; i++) {
			assets.loadppm(eng.ren.pathprefix + imagespath[i]);
		}
		std::cout << "log: main textures loaded" << std::endl;
		for (int i = 0; i != cubescount * 6; i++) {
			cube.loadppm(eng.ren.pathprefix + cubespath[i]);
		}
		std::cout << "log: cubemaps loaded" << std::endl;
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), assets.pixels.data(), assets.textureResolution, imagecount, cube.pixels.data(), cube.textureResolution, cubescount);
		std::cout << "log: object created" << std::endl;
		withaudio = true;
	}
	void createNoCube(Engine& eng, std::string vertshader, std::string fragshader, std::string modelpath, std::string* imagespath, int imagecount, std::string audiopath, float maxdist) {
		audio.maxdist = maxdist;
		audio.create(eng.aud, eng.ren.pathprefix + audiopath);
		assets.loadobj(eng.ren.pathprefix + modelpath);
		std::cout << "log: model loaded" << std::endl;
		for (int i = 0; i != imagecount; i++) {
			assets.loadppm(eng.ren.pathprefix + imagespath[i]);
		}
		std::cout << "log: main textures loaded" << std::endl;
		unsigned char data[24];
		for (int i = 0; i != 24; i++) {
			data[i] = 0;
		}
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), assets.pixels.data(), assets.textureResolution, imagecount, data, glm::ivec2(1, 1), 1);
		std::cout << "log: object created" << std::endl;
		withaudio = true;
	}
	void createNoTex(Engine& eng, std::string vertshader, std::string fragshader, std::string modelpath, std::string* cubespath, int cubescount, std::string audiopath, float maxdist) {
		audio.maxdist = maxdist;
		audio.create(eng.aud, eng.ren.pathprefix + audiopath);
		assets.loadobj(eng.ren.pathprefix + modelpath);
		std::cout << "log: model loaded" << std::endl;
		unsigned char data[4] = { 0, 0, 0, 0 };
		for (int i = 0; i != cubescount * 6; i++) {
			cube.loadppm(eng.ren.pathprefix + cubespath[i]);
		}
		std::cout << "log: cubemaps loaded" << std::endl;
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), data, glm::ivec2(1, 1), 1, cube.pixels.data(), cube.textureResolution, cubescount);
		std::cout << "log: object created" << std::endl;
		withaudio = true;
	}
	void applyChanges(Engine& eng) {
		mesh.applyChanges(eng.ren);
	}
	void Draw(Engine& eng) {
		mesh.pos = pos;
		audio.pos = pos;
		mesh.rot = rot;
		mesh.scale = scale;
		if (withaudio) {
			audio.play(eng.volume, eng.ren.pos);
		}
		mesh.Draw(eng.ren);
	}
};