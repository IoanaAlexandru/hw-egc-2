#ifndef POOL_SHADOW_MAP_H_
#define POOL_SHADOW_MAP_H_

#include <string>
#include <vector>
#include <list>
#include <functional>

#include <include/gl.h>

namespace pool {
class ShadowMapFBO {
public:
	ShadowMapFBO();
	~ShadowMapFBO();

	bool Init(unsigned int width, unsigned int height);
	void BindForWriting();
	void BindForReading(GLenum textureUnit);
	void Clean();

private:
	GLuint m_fbo;
	GLuint m_shadowMap;
};
}
#endif	// POOL_SHADOW_MAP_H_
