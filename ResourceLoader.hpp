#include <iostream>
#include <fstream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <string>
#include <vector>
#include <array>
#include <cstring>

class Loader {
public:
	std::vector<glm::vec3> vertex;
	std::vector<glm::vec2> uv;
	std::vector<glm::vec3> normals;
	std::vector<unsigned char> pixels;
	glm::ivec2 textureResolution = glm::ivec2(0, 0);
	void loadppm(std::string path) {
		std::fstream readimage;
		readimage.open(path);
		int i1, i2, i3;
		char c;
		std::string format;
		readimage >> format;
		readimage >> textureResolution.x >> textureResolution.y;
		readimage >> i1;
		pixels.resize(textureResolution.x * textureResolution.y * 4);
		if (format == "P3") {
			for (int i = 0; readimage >> i1 >> i2 >> i3; i += 4) {
				pixels[i] = i1;
				pixels[i + 1] = i2;
				pixels[i + 2] = i3;
				pixels[i + 3] = 256;
			}
		}
		else {
			readimage.get(c);
			for (int i = 0; i != textureResolution.x * textureResolution.y * 4; i += 4) {
				readimage.get(c);
				pixels[i] = c;
				readimage.get(c);
				pixels[i + 1] = c;
				readimage.get(c);
				pixels[i + 2] = c;
				pixels[i + 3] = 256;
			}
		}
		readimage.close();
	}
	void loadobj(std::string path) {
		std::vector<glm::vec4> lvertex;
		std::vector<glm::vec3> lnormals;
		std::vector<glm::vec2> luv;
		std::vector<int> indices;
		std::vector<int> nindices;
		std::vector<int> uindices;
		FILE* obj;
		//fopen(obj, path, "r");
		obj = fopen(path.c_str(), "r");
		std::string arg;
		int line = 1;
		int nline = 1;
		int uvline = 1;
		int faceline = 0;
		while (1) {
			char lineHeader[128];

			if (obj == 0) {
				exit(-1);
			}

			int res = fscanf(obj, "%s", lineHeader);
			if (res == EOF) {
				break;
			}

			if (strcmp(lineHeader, "#") == 0) {
				fscanf(obj, "%*[^\n]\n");
			}

			if (strcmp(lineHeader, "s") == 0) {
				fscanf(obj, "%*[^\n]\n");
			}

			if (strcmp(lineHeader, "o") == 0) {
				fscanf(obj, "%*[^\n]\n");
			}

			if (strcmp(lineHeader, "v") == 0) {
				lvertex.resize(lvertex.size() + 2);
				fscanf(obj, "%f %f %f \n", &lvertex[line].x, &lvertex[line].y, &lvertex[line].z);
				lvertex[line].w = 1;
				line++;
			}

			if (strcmp(lineHeader, "vn") == 0) {
				lnormals.resize(lnormals.size() + 2);
				fscanf(obj, "%f %f %f \n", &lnormals[nline].x, &lnormals[nline].y, &lnormals[nline].z);
				nline++;
			}

			if (strcmp(lineHeader, "vt") == 0) {
				luv.resize(luv.size() + 2);
				fscanf(obj, "%f %f \n", &luv[uvline].x, &luv[uvline].y);
				uvline++;
			}

			if (strcmp(lineHeader, "f") == 0) {
				indices.resize(indices.size() + 3);
				uindices.resize(uindices.size() + 3);
				nindices.resize(nindices.size() + 3);
				fscanf(obj, "%d/%d/%d %d/%d/%d %d/%d/%d \n", &indices[faceline], &uindices[faceline], &nindices[faceline], &indices[faceline + 1], &uindices[faceline + 1], &nindices[faceline + 1], &indices[faceline + 2], &uindices[faceline + 2], &nindices[faceline + 2]);
				faceline = faceline + 3;
			}
		}
		fclose(obj);
		vertex.resize(indices.size() * 3);
		normals.resize(indices.size() * 3);
		uv.resize(indices.size() * 3);
		for (int i = 0; i != faceline; i++) {
			vertex[i].x = lvertex[indices[i]].x;
			vertex[i].y = lvertex[indices[i]].y;
			vertex[i].z = lvertex[indices[i]].z;

			normals[i].x = lnormals[nindices[i]].x;
			normals[i].y = lnormals[nindices[i]].y;
			normals[i].z = lnormals[nindices[i]].z;

			uv[i].x = luv[uindices[i]].x;
			uv[i].y = luv[uindices[i]].y;
		}
	}
};