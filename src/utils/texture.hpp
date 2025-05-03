#ifndef _UTILS_TEXTURE_H
#define _UTILS_TEXTURE_H

#include <sstream>
#include <string>
#include <vector>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "gl_utility.hpp"

class Texture {
public:
    Texture();

    Texture(Texture&& rhs) noexcept;

    virtual ~Texture();

    virtual void bind(int slot = 0) const = 0;

    virtual void unbind() const = 0;

    virtual void generateMipmap() const = 0;

    virtual void setParamterInt(GLenum name, int value) const = 0;

    GLuint getHandle() const;

protected:
    GLuint _handle = {};

    void check();

    virtual void cleanup();
};

#endif