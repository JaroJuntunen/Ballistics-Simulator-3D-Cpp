#include "ShaderProgram.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

ShaderProgram::~ShaderProgram() {
	if (m_id) glDeleteProgram(m_id);
}

bool ShaderProgram::load(const std::string& vertPath, const std::string& fragPath) {
	std::string vertSrc = readFile(vertPath);
	std::string fragSrc = readFile(fragPath);

	if (vertSrc.empty() || fragSrc.empty()) {
		std::cerr << "ShaderProgram: failed to read shader files\n"
				  << "  vert: " << vertPath << "\n"
				  << "  frag: " << fragPath << "\n";
		return false;
	}

	unsigned int vert = compileShader(GL_VERTEX_SHADER,   vertSrc);
	unsigned int frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

	if (!vert || !frag) {
		glDeleteShader(vert);
		glDeleteShader(frag);
		return false;
	}

	m_id = glCreateProgram();
	glAttachShader(m_id, vert);
	glAttachShader(m_id, frag);
	glLinkProgram(m_id);

	int ok;
	glGetProgramiv(m_id, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetProgramInfoLog(m_id, 512, nullptr, log);
		std::cerr << "ShaderProgram: link error:\n" << log << "\n";
		glDeleteProgram(m_id);
		m_id = 0;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
	return m_id != 0;
}

void ShaderProgram::bind()   const { glUseProgram(m_id); }
void ShaderProgram::unbind() const { glUseProgram(0);    }

void ShaderProgram::setMat4(const std::string& name, const glm::mat4& v) const {
	glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(v));
}

void ShaderProgram::setVec3(const std::string& name, const glm::vec3& v) const {
	glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(v));
}

void ShaderProgram::setFloat(const std::string& name, float v) const {
	glUniform1f(glGetUniformLocation(m_id, name.c_str()), v);
}

// ── Private helpers ───────────────────────────────────────────────────────────

unsigned int ShaderProgram::compileShader(unsigned int type, const std::string& src) {
	unsigned int id = glCreateShader(type);
	const char*  c  = src.c_str();
	glShaderSource(id, 1, &c, nullptr);
	glCompileShader(id);

	int ok;
	glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetShaderInfoLog(id, 512, nullptr, log);
		std::cerr << "ShaderProgram: compile error ("
				  << (type == GL_VERTEX_SHADER ? "vert" : "frag") << "):\n"
				  << log << "\n";
		glDeleteShader(id);
		return 0;
	}
	return id;
}

std::string ShaderProgram::readFile(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) return {};
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}
