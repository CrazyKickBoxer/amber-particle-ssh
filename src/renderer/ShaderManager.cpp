#include "ShaderManager.h"
#include <QFile>
#include <QDebug>
#include <QCoreApplication>

static QString resolvePath(const QString& relativePath) {
    // Try relative to executable first
    QString exePath = QCoreApplication::applicationDirPath() + "/" + relativePath;
    if (QFile::exists(exePath)) return exePath;
    
    // Fallback to current directory
    return relativePath;
}

std::unique_ptr<QOpenGLShaderProgram> ShaderManager::createProgram(const QString& name, 
                                                                 const QString& vertPath, 
                                                                 const QString& fragPath)
{
    auto program = std::make_unique<QOpenGLShaderProgram>();
    
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, resolvePath(vertPath))) {
        qWarning() << "Failed to compile vertex shader for" << name << ":" << program->log();
        return nullptr;
    }
    
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, resolvePath(fragPath))) {
        qWarning() << "Failed to compile fragment shader for" << name << ":" << program->log();
        return nullptr;
    }
    
    if (!program->link()) {
        qWarning() << "Failed to link shader program" << name << ":" << program->log();
        return nullptr;
    }
    
    return program;
}

std::unique_ptr<QOpenGLShaderProgram> ShaderManager::createComputeProgram(const QString& name,
                                                                        const QString& computePath)
{
    auto program = std::make_unique<QOpenGLShaderProgram>();
    
    if (!program->addShaderFromSourceFile(QOpenGLShader::Compute, resolvePath(computePath))) {
        qWarning() << "Failed to compile compute shader for" << name << ":" << program->log();
        return nullptr;
    }
    
    if (!program->link()) {
        qWarning() << "Failed to link compute program" << name << ":" << program->log();
        return nullptr;
    }
    
    return program;
}
