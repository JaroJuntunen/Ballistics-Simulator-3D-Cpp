#pragma once

#include <glm/glm.hpp>
#include <string>

// Compiles and links a GLSL vertex + fragment shader pair.
// Loaded from files relative to the executable's working directory.
class ShaderProgram {
public:
	~ShaderProgram();

	bool load(const std::string& vertPath, const std::string& fragPath);
	void bind()   const;
	void unbind() const;

	void setMat4 (const std::string& name, const glm::mat4& v) const;
	void setVec3 (const std::string& name, const glm::vec3& v) const;
	void setFloat(const std::string& name, float v)            const;

private:
	unsigned int m_id = 0;

	static unsigned int compileShader(unsigned int type, const std::string& src);
	static std::string  readFile(const std::string& path);
};
