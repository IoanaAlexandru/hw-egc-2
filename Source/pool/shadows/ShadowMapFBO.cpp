#include "ShadowMapFBO.h"
#include <iostream>
using namespace std;

namespace pool {
	ShadowMapFBO::ShadowMapFBO()
	{
		m_fbo = 0;
	}

	ShadowMapFBO::~ShadowMapFBO(){};

	void ShadowMapFBO::Clean()
	{
		if (m_fbo)
		{
			glDeleteFramebuffers(1, &m_fbo);
		}
	}

	bool ShadowMapFBO::Init(unsigned int width, unsigned int height)
	{
		Clean();
		// Create frame buffer object
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		// Create depth texture
		glGenTextures(1, &m_shadowMap);
		glBindTexture(GL_TEXTURE_2D, m_shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Attach shadow map texture to the depth attachment of FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);

		// Render only to depth buffer
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		// Verify fbo configuration
		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE) {
			cout << "FB error";
			return false;
		}
	}
	void ShadowMapFBO::BindForWriting()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	}
	void ShadowMapFBO::BindForReading(GLenum textureUnit)
	{
		glActiveTexture(textureUnit);
		glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	}
} // namespace pool