#pragma once
#include "PostProcessRenderer.h"
#include <glm/glm.hpp>
#include <GL/glew.h>

class PulseEffectRenderer : public PostProcessRenderer
{
public:
    glm::vec2 pulseCenter = { 0.f, 0.f };
    float pulseTime = 0.f;
    float pulseRadius = 0.1f;
    float pulseStrength = 0.5f;
    float pulseWidth = 0.2f;
	bool pulseActive = false;

	int screenWidth = 800;
	int screenHeight = 800;

public:
    PulseEffectRenderer(GLuint shaderProgram)
        : PostProcessRenderer(shaderProgram)
    {
    }

    virtual void passUniforms() override
    {
        GLint locCenter = glGetUniformLocation(shader, "uPulseCenter");
        GLint locTime = glGetUniformLocation(shader, "uPulseTime");
        GLint locRadius = glGetUniformLocation(shader, "uPulseRadius");
        GLint locStrength = glGetUniformLocation(shader, "uPulseStrength");
        GLint locWidth = glGetUniformLocation(shader, "uPulseWidth");
        GLint locActive = glGetUniformLocation(shader, "uPulseActive");

        GLint locScreen = glGetUniformLocation(shader, "uResolution");

        glm::vec2 centerWithNegY = glm::vec2(pulseCenter.x, pulseCenter.y);
        glUniform2fv(locCenter, 1, &pulseCenter.x);

        glUniform1f(locTime, pulseTime);
        glUniform1f(locRadius, pulseRadius);
        glUniform1f(locStrength, pulseStrength);
        glUniform1f(locWidth, pulseWidth);
        glUniform1i(locActive, pulseActive ? 1 : 0);

		glUniform2f(locScreen, (float)screenWidth, (float)screenHeight);
    }
};
