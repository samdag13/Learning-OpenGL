#pragma once
#include "Renderer.h"

class Texture {
private:
	unsigned int m_RendererID;
	std::string m_Filepath;
	unsigned char* m_localBuffer;
	int m_Width, m_Heigth, m_BPP;


public:
	Texture(const std::string& path);
	~Texture();
	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int getWidth() const { return m_Width; }
	inline int getHeigth() const { return m_Heigth; }
};