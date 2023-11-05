#include <iostream>

#include "Render.hpp"

#include "Audio.hpp"

#include "Physics.hpp"

#include "clickzone.hpp"

class Engine {
private:
	ma_result result;
	bool begun = false;
#if defined(__ANDROID__)
	ANativeWindow* lwindow;
#endif
public:
	glm::vec3 pos = glm::vec3(0, 0, 0);
	glm::vec2 rot = glm::vec2(0, 0);
	ma_engine aud;
	Render ren;
	PhysEngine peng;
	bool enablefallphysics = true;
	float volume = 1.0f;
#if defined(__ANDROID__)
	void init(std::string appname, ANativeWindow* window) {
		lwindow = window;
		ren.init(appname, window);
		result = ma_engine_init(NULL, &aud);
		if (result != MA_SUCCESS) {
			exit(result);
		}
		ren.pos = pos;
		ren.rot = rot;
		peng.pos = pos;
		peng.lpos = pos;
	}
#else
	void init(std::string appname) {
		ren.init(appname);
		result = ma_engine_init(NULL, &aud);
		if (result != MA_SUCCESS) {
			std::cout << "error:\u001b[31m failed to init miniaudio\u001b[37m" << std::endl;
			exit(-1);
		}
		ren.pos = pos;
		ren.rot = rot;
		peng.pos = pos;
		peng.lpos = pos;
		std::cout << "log:\u001b[32m engine inited with success\u001b[37m" << std::endl;
	}
#endif
	void beginShadowPass() {
		if (!begun) {
#if defined(__ANDROID__)
			ren.beginRender(lwindow);
#else
			ren.beginRender();
#endif
			ren.pos = pos;
			ren.rot = rot;
			peng.pos = pos;
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
			ren.pos = pos;
			ren.rot = rot;
			peng.pos = pos;
			begun = true;
		}
		ren.beginMainPass();
	}
	void endRender() {
		ren.endRender();
		for (int i = 0; i != peng.bombs.size(); i++) {
			peng.bombs[i].actioned = true;
		}
		begun = false;
		peng.lpos = peng.pos;
		ren.pos = peng.pos;
		pos = peng.pos;
		if (enablefallphysics) {
			pos.y -= peng.mass;
		}
	}
	void terminate() {
		ren.terminate();
	}
};

class Object {
private:
	Loader assets{};
	Loader cube{};
public:
	Mesh mesh;
	MeshPhys phys;
	AudioSource audio;
	glm::vec3 pos = glm::vec3(0, 0, 0);
	glm::vec3 rot = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	bool enablecollisiondetect = true;
	bool withaudio = false;
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
		unsigned char data[24]{};
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
		unsigned char data[24]{};
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), assets.pixels.data(), assets.textureResolution, imagecount, data, glm::ivec2(1, 1), 1);
		std::cout << "log: object created" << std::endl;
		withaudio = true;
	}
	void createNoTex(Engine& eng, std::string vertshader, std::string fragshader, std::string modelpath, std::string* cubespath, int cubescount, std::string audiopath, float maxdist) {
		audio.maxdist = maxdist;
		audio.create(eng.aud, eng.ren.pathprefix + audiopath);
		assets.loadobj(eng.ren.pathprefix + modelpath);
		std::cout << "log:\u001b[36m model loaded\u001b[32m" << std::endl;
		unsigned char data[4] = { 0, 0, 0, 0 };
		for (int i = 0; i != cubescount * 6; i++) {
			cube.loadppm(eng.ren.pathprefix + cubespath[i]);
		}
		std::cout << "log:\u001b[36m cubemaps loaded\u001b[32m" << std::endl;
		mesh.create(eng.ren, eng.ren.pathprefix + vertshader, eng.ren.pathprefix + fragshader, assets.vertex.data(), assets.uv.data(), assets.normals.data(), assets.vertex.size(), data, glm::ivec2(1, 1), 1, cube.pixels.data(), cube.textureResolution, cubescount);
		std::cout << "log:\u001b[32m object created\u001b[32m" << std::endl;
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
		if (enablecollisiondetect && !eng.ren.shadowpass) {
			for (int i = 0; i != mesh.vertexdata.size(); i += 3) {
				phys.physWork(eng.peng, mesh.vertexdata[i].position, mesh.vertexdata[i + 1].position, mesh.vertexdata[i + 2].position, eng.ren.ubo.mtranslate, eng.ren.ubo.mrotx, eng.ren.ubo.mroty, eng.ren.ubo.mrotz, eng.ren.ubo.mscale);
			}
		}
	}
};

class uiBanner {
private:
	Loader assets{};
public:
	Object banner;
	glm::vec2 pos = glm::vec2(0, 0);
	glm::vec2 size = glm::vec2(0, 0);
	float depth = 0.5;
	void create(Engine& eng, glm::vec2 bpos, glm::vec2 bsize, glm::vec2 *uv, std::string pathtotex, std::string vertshader, std::string fragshader) {
		pos = bpos;
		size = bsize;
		banner.pos = glm::vec3(bpos.x, bpos.y, -depth);
		banner.mesh.cullmode = VK_CULL_MODE_NONE;
		banner.enablecollisiondetect = false;
		glm::vec3 vert[6] = {
			glm::vec3(0, 0 + size.y, 0),
			glm::vec3(0, 0, 0),
			glm::vec3(0 + size.y, 0, 0),
			glm::vec3(0, 0 + size.y, 0),
			glm::vec3(0 + size.y, 0, 0),
			glm::vec3(0 + size.y, 0 + size.y, 0)
		};
		glm::vec2 luv[6] = {
			uv[0],
			uv[1],
			uv[2],
			uv[0],
			uv[2],
			uv[3]
		};
		assets.loadppm(pathtotex);
		unsigned char c[24]{};
		banner.create(eng, vertshader, fragshader, vert, luv, vert, 6, assets.pixels.data(), assets.textureResolution, 1, c, glm::ivec2(1, 1), 1);
	}
	void create(Engine& eng, glm::vec2 bpos, glm::vec2 bsize, std::string pathtotex, std::string vertshader, std::string fragshader) {
		pos = bpos;
		size = bsize;
		banner.pos = glm::vec3(bpos.x, bpos.y, -depth);
		banner.mesh.cullmode = VK_CULL_MODE_NONE;
		banner.enablecollisiondetect = false;
		glm::vec3 vert[6] = {
			glm::vec3(0, 0 + size.y, 0),
			glm::vec3(0, 0, 0),
			glm::vec3(0 + size.y, 0, 0),
			glm::vec3(0, 0 + size.y, 0),
			glm::vec3(0 + size.y, 0, 0),
			glm::vec3(0 + size.y, 0 + size.y, 0)
		};
		glm::vec2 luv[6] = {
			glm::vec2(0, 1),
			glm::vec2(0, 0),
			glm::vec2(1, 0),
			glm::vec2(0, 1),
			glm::vec2(1, 0),
			glm::vec2(1, 1)
		};
		assets.loadppm(pathtotex);
		unsigned char c[24]{};
		banner.create(eng, vertshader, fragshader, vert, luv, vert, 6, assets.pixels.data(), assets.textureResolution, 1, c, glm::ivec2(1, 1), 1);
	}
	void Draw(Engine& eng) {
		eng.ren.uimat = true;
		banner.pos = glm::vec3(pos.x, pos.y, -depth);
		banner.Draw(eng);
		eng.ren.uimat = false;
	}
};

class uiButton {
private:
	float aspect = 1.0f;
public:
	uiBanner button;
	clickzone zone;
	glm::vec2 pos = glm::vec2(0, 0);
	glm::vec2 size = glm::vec2(0, 0);
	void create(Engine& eng, glm::vec2 bpos, glm::vec2 bsize, glm::vec2* uv, std::string pathtotex, std::string vertshader, std::string fragshader) {
		button.create(eng, bpos, bsize, uv, pathtotex, vertshader, fragshader);
		button.pos = bpos;
		button.size = bsize;
		zone.pos = bpos;
		zone.size = bsize;
		pos = bpos;
		size = bsize;
	}
	void create(Engine& eng, glm::vec2 bpos, glm::vec2 bsize, std::string pathtotex, std::string vertshader, std::string fragshader) {
		button.create(eng, bpos, bsize, pathtotex, vertshader, fragshader);
		button.pos = bpos;
		button.size = bsize;
		zone.pos = bpos;
		zone.size = bsize;
		pos = bpos;
		size = bsize;
	}
	int Draw(Engine& eng, glm::vec2 pointer, bool ispressed) {
		aspect = eng.ren.resolution.x / eng.ren.resolution.y;
		button.pos = pos;
		button.size = size;
		zone.pos = pos;
		zone.size = size;
		button.Draw(eng);
		return zone.action(glm::vec2(pointer.x, pointer.y), ispressed);
	}
};