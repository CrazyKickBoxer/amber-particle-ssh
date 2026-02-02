#pragma once

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QString>
#include <memory>

class ShaderManager
{
public:
    static std::unique_ptr<QOpenGLShaderProgram> createProgram(const QString& name, 
                                                             const QString& vertPath, 
                                                             const QString& fragPath);
    
    static std::unique_ptr<QOpenGLShaderProgram> createComputeProgram(const QString& name,
                                                                    const QString& computePath);
};
