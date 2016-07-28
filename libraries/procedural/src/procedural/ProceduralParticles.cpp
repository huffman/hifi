//
//  Created by Sam Gondelman on 7/20/16
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ProceduralParticles.h"

#include "glm/glm.hpp"

#include <gpu/Batch.h>
#include <gpu/Framebuffer.h>
#include <FramebufferCache.h>

#include <DependencyManager.h>

#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>

#include "procedural_particle_update_frag.h"
#include "draw_procedural_particle_vert.h"
#include "untextured_procedural_particle_frag.h"
#include "textured_procedural_particle_frag.h"
#include <gpu/DrawUnitQuadTexcoord_vert.h>

static const QString PROCEDURAL_USER_DATA_KEY = "ProceduralParticles";
static const QString URL_KEY = "shaderUrl";
static const QString UNIFORMS_KEY = "uniforms";

static int UPDATE_PARTICLES_BUFFER = -1;
static int UPDATE_HIFI_BUFFER = -1;
static int UPDATE_PARTICLES = -1;
static int NOTEX_DRAW_BUFFER = -1;
static int NOTEX_DRAW_PARTICLES = -1;
static int DRAW_BUFFER = -1;
static int DRAW_PARTICLES = 0;
static int DRAW_TEXTURE = 1;

static std::string HIFI_UPDATE_UNIFORMS = "#define HIFI_UPDATE_UNIFORMS";
static QString BEGIN_HIFI_UPDATE_UNIFORMS = "#BEGIN_HIFI_UPDATE_UNIFORMS";
static QString END_HIFI_UPDATE_UNIFORMS = "#END_HIFI_UPDATE_UNIFORMS";

static std::string HIFI_UPDATE_METHODS = "#define HIFI_UPDATE_METHODS";
static QString BEGIN_HIFI_UPDATE_METHODS = "#BEGIN_HIFI_UPDATE_METHODS";
static QString END_HIFI_UPDATE_METHODS = "#END_HIFI_UPDATE_METHODS";

static std::string HIFI_DRAW_METHODS = "#define HIFI_DRAW_METHODS";
static QString BEGIN_HIFI_DRAW_METHODS = "#BEGIN_HIFI_DRAW_METHODS";
static QString END_HIFI_DRAW_METHODS = "#END_HIFI_DRAW_METHODS";

ProceduralParticles::ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM) {
    _particleUniforms = ParticleUniforms();
    _particleUniforms.color = color;
    _particleUniforms.radius = radius;
    _particleUniforms.iResolution.z = maxParticles;

    // Create the FBOs
    for (int i = 0; i < 2; i++) {
        _particleBuffers.append(gpu::FramebufferPointer(gpu::Framebuffer::create(gpu::Format(gpu::Dimension::VEC4,
                                gpu::Type::FLOAT,
                                gpu::Semantic::RGBA), 1, 1)));
    }

    // Resize the FBOs
    setMaxParticles(maxParticles, MAX_DIM);

    _uniformBuffer = std::make_shared<Buffer>(sizeof(ParticleUniforms), (const gpu::Byte*) &_particleUniforms);

    _proceduralDataDirty = false;
}

ProceduralParticles::ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM, const QString& userDataJson) :
    ProceduralParticles(color, radius, maxParticles, MAX_DIM)
{
    setUserData(userDataJson);
    _proceduralDataDirty = true;
}

ProceduralParticles::ProceduralParticles(glm::vec4 color, float radius, quint32 maxParticles, quint32 MAX_DIM, const QVariantMap& userDataJson) :
    ProceduralParticles(color, radius, maxParticles, MAX_DIM)
{
    setUserData(userDataJson);
    _proceduralDataDirty = true;
}

// Example
// {"ProceduralParticles":
//   {"shaderUrl": "https://hifi-content.s3.amazonaws.com/samuel/loadingParticles.fs",
//    "uniforms": [{"numObjects":2}, {"objects":[[5, 5, 5, 0], [1, 1, 1, 0], [10, 10, 10, 0], [3, 3, 3, 0]]}]
//   }
// };
void ProceduralParticles::setUserData(const QString& userDataJson) {
    auto proceduralData = getProceduralData(userDataJson);

    std::lock_guard<std::mutex> lock(_proceduralDataMutex);
    _proceduralData = proceduralData.toObject();

    // Mark as dirty after modifying _proceduralData, but before releasing lock
    // to avoid setting it after parsing has begun
    _proceduralDataDirty = true;
}

void ProceduralParticles::setUserData(const QVariantMap& userDataJson) {
    std::lock_guard<std::mutex> lock(_proceduralDataMutex);
    _proceduralData = QJsonObject::fromVariantMap(userDataJson[PROCEDURAL_USER_DATA_KEY].toMap());

    // Mark as dirty after modifying _proceduralData, but before releasing lock
    // to avoid setting it after parsing has begun
    _proceduralDataDirty = true;
}

QJsonValue ProceduralParticles::getProceduralData(const QString& proceduralJson) {
    if (proceduralJson.isEmpty()) {
        return QJsonValue();
    }

    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(proceduralJson.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return QJsonValue();
    }

    return doc.object()[PROCEDURAL_USER_DATA_KEY];
}

bool ProceduralParticles::parseUrl(const QUrl& shaderUrl) {
    if (!shaderUrl.isValid()) {
        if (!shaderUrl.isEmpty()) {
            qWarning() << "Invalid shader URL: " << shaderUrl;
        }
        _networkShader.reset();
        return false;
    } else if (shaderUrl == _shaderUrl) {
        return true;
    }

    _shaderUrl = shaderUrl;
    _shaderDirty = true;
    _shadersCompiled = true;

    if (_shaderUrl.isLocalFile()) {
        _shaderPath = _shaderUrl.toLocalFile();
        qDebug() << "Shader path: " << _shaderPath;
        if (!QFile(_shaderPath).exists()) {
            _networkShader.reset();
            return false;;
        }
    } else {
        qDebug() << "Shader url: " << _shaderUrl;
        _networkShader = ShaderCache::instance().getShader(_shaderUrl);
    }

    return true;
}

bool ProceduralParticles::parseUniforms(const QJsonArray& uniforms) {
    if (_parsedUniforms != uniforms) {
        _parsedUniforms = uniforms;
        _uniformsDirty = true;
    }

    return true;
}

void ProceduralParticles::parse(const QJsonObject& proceduralData) {
    _enabled = false;

    auto shaderUrl = proceduralData[URL_KEY].toString();
    shaderUrl = ResourceManager::normalizeURL(shaderUrl);
    auto uniforms = proceduralData[UNIFORMS_KEY].toArray();

    bool isValid = true;

    // Run through parsing regardless of validity to clear old cached resources
    isValid = parseUrl(shaderUrl) && isValid;
    isValid = parseUniforms(uniforms) && isValid;

    if (!proceduralData.isEmpty() && isValid) {
        _enabled = true;
    }
}

void ProceduralParticles::setMaxParticles(quint32 maxParticles, const quint32 MAX_DIM) {
    _particleUniforms.iResolution.z = maxParticles;

    int width = std::min(MAX_DIM, maxParticles);
    int height = ((int)(maxParticles / MAX_DIM) + 1) * 2;
    for (auto& buffer : _particleBuffers) {
        buffer->resize(width, height);
    }

    _particleUniforms.iResolution.x = width;
    _particleUniforms.iResolution.y = height;

    // Restart the simulation when the number of particles changes
    _particleUniforms.firstPass = true;
}

void ProceduralParticles::setTextures(const QString& textures) {
    if (textures.isEmpty()) {
        _texture.clear();
    } else {
        _texture = DependencyManager::get<TextureCache>()->getTexture(textures);
    }
}

void ProceduralParticles::update(float iGlobalTime, float iDeltaTime) {
    _particleUniforms.iGlobalTime = iGlobalTime;
    _particleUniforms.iDeltaTime = iDeltaTime;

    // TODO: remove (for testing)
    if (!_texture) {
        _texture = DependencyManager::get<TextureCache>()->getTexture(QString("https://hifi-public.s3.amazonaws.com/alan/Particles/Particle-Sprite-Smoke-1.png"));
    }
}

bool ProceduralParticles::ready() {
    // Load any changes to the procedural
    // Check for changes atomically, in case they are currently being made
    if (_proceduralDataDirty) {
        std::lock_guard<std::mutex> lock(_proceduralDataMutex);
        parse(_proceduralData);

        // Reset dirty flag after reading _proceduralData, but before releasing lock
        // to avoid resetting it after more data is set
        _proceduralDataDirty = false;
    }

    if (!_enabled) {
        return false;
    }

    // Do we have a network or local shader, and if so, is it loaded?
    if (_shaderPath.isEmpty() && (!_networkShader || !_networkShader->isLoaded())) {
        return false;
    }

    // Have we already tried compiling the shaders, but the compilation failed?
    return _shadersCompiled;
}

void ProceduralParticles::setupUniforms() {
    // Always setup the particle buffer so the particle uniforms are updated
    memcpy(&editParticleUniforms(), &_particleUniforms, sizeof(ParticleUniforms));

    if (_shaderDirty || _uniformsDirty) {
        // Set any userdata specified uniforms
        // The order of the uniforms must be preserved, so everything is stored in a larger QJsonArray
        int index = 0;
        foreach(auto object, _parsedUniforms) {
            QJsonObject uniform = object.toObject();
            foreach(auto key, uniform.keys()) {
                QJsonValue value =  uniform[key];

                if (value.isDouble()) {                                     // floats, ints
                    _hifiBufferData[index] = value.toDouble();
                    index++;
                } else if (value.isArray()) {                               // arrays
                    auto valueArray = value.toArray();

                    foreach(auto innerValue, valueArray) {
                        if (innerValue.isDouble()) {                        // float[], int[]
                            _hifiBufferData[index] = innerValue.toDouble();
                            index++;
                        } else if (innerValue.isArray()) {                  // vec2[], vec3[], vec4[]
                            auto innerValueArray = innerValue.toArray();

                            foreach(auto v, innerValueArray) {
                                _hifiBufferData[index] = v.toDouble();
                                index++;
                            }
                        }
                    }
                }
            }
        }

        // Only send as much data as we're using
        memcpy(&editHifiUniforms(index), &_hifiBufferData[0], index * sizeof(float));
    }
}

void ProceduralParticles::render(RenderArgs* args) {
    if (_shaderUrl.isLocalFile()) {
        auto lastModified = (quint64)QFileInfo(_shaderPath).lastModified().toMSecsSinceEpoch();
        if (lastModified > _shaderModified) {
            QFile file(_shaderPath);
            file.open(QIODevice::ReadOnly);
            _shaderSource = QTextStream(&file).readAll();
            _shaderDirty = true;
            _shaderModified = lastModified;
        }
    } else if (_networkShader && _networkShader->isLoaded()) {
        _shaderSource = _networkShader->_source;
    }

    // lazy creation of particle system pipeline
    if (_shaderDirty) {
        createPipelines();
        if (!_shadersCompiled) {
            return;
        }
    }

    auto mainViewport = args->_viewport;
    gpu::Batch& batch = *args->_batch;

    setupUniforms();

    _shaderDirty = _uniformsDirty = false;

    // Update the particles in the other FBO based on the current FBO's texture
    batch.setPipeline(_updatePipeline);
    batch.setUniformBuffer(UPDATE_PARTICLES_BUFFER, _uniformBuffer);
    batch.setUniformBuffer(UPDATE_HIFI_BUFFER, _hifiBuffer);
    batch.setFramebuffer(_particleBuffers[(int)!_evenPass]);
    glm::ivec4 viewport = glm::ivec4(0, 0, _particleBuffers[(int)!_evenPass]->getWidth(), _particleBuffers[(int)!_evenPass]->getHeight());
    batch.setViewportTransform(viewport);
    batch.setResourceTexture(UPDATE_PARTICLES, _particleBuffers[(int)_evenPass]->getRenderBuffer(0));
    batch.draw(gpu::TRIANGLE_STRIP, 4);

    // Render using the updated FBO's texture
    auto lightingFramebuffer = DependencyManager::get<FramebufferCache>()->getLightingFramebuffer();
    batch.setFramebuffer(lightingFramebuffer);
    batch.setViewportTransform(mainViewport);
    if (_texture && _texture->isLoaded()) {
        batch.setPipeline(_texturedPipeline);
        batch.setUniformBuffer(DRAW_BUFFER, _uniformBuffer);
        batch.setResourceTexture(DRAW_PARTICLES, _particleBuffers[(int)!_evenPass]->getRenderBuffer(0));
        batch.setResourceTexture(DRAW_TEXTURE, _texture->getGPUTexture());
    } else {
        batch.setPipeline(_untexturedPipeline);
        batch.setUniformBuffer(NOTEX_DRAW_BUFFER, _uniformBuffer);
        batch.setResourceTexture(NOTEX_DRAW_PARTICLES, _particleBuffers[(int)!_evenPass]->getRenderBuffer(0));
    }

    batch.setModelTransform(Transform());

    batch.draw(gpu::TRIANGLES, VERTEX_PER_PARTICLE * _particleUniforms.iResolution.z);
    //batch.drawInstanced((gpu::uint32)_particleUniforms.iResolution.z, gpu::TRIANGLES, (gpu::uint32)VERTEX_PER_PARTICLE);

    _particleUniforms.firstPass = false;
    _evenPass = !_evenPass;
}

void ProceduralParticles::createPipelines() {
    bool success = true;

    const std::string particleBuffer = "particleBuffer";
    const std::string hifiBuffer = "hifiBuffer";
    const std::string particlesTex = "particlesTex";
    const std::string colorMap = "colorMap";

    // Update pipeline
    auto state = std::make_shared<gpu::State>();
    state->setCullMode(gpu::State::CULL_BACK);

    std::string frag = procedural_particle_update_frag;

    int uniformsIndex = frag.find(HIFI_UPDATE_UNIFORMS);
    int uniformsBegin = _shaderSource.indexOf(BEGIN_HIFI_UPDATE_UNIFORMS) + BEGIN_HIFI_UPDATE_UNIFORMS.length();
    int uniformsEnd = _shaderSource.indexOf(END_HIFI_UPDATE_UNIFORMS);

    if (uniformsIndex != -1 && uniformsBegin != -1 && uniformsEnd != -1) {
        frag.replace(uniformsIndex, HIFI_UPDATE_UNIFORMS.length(),
                     _shaderSource.mid(uniformsBegin, uniformsEnd - uniformsBegin).toStdString());
    } else {
        success = false;
    }

    int methodsIndex = frag.find(HIFI_UPDATE_METHODS);
    int methodsBegin = _shaderSource.indexOf(BEGIN_HIFI_UPDATE_METHODS) + BEGIN_HIFI_UPDATE_METHODS.length();
    int methodsEnd = _shaderSource.indexOf(END_HIFI_UPDATE_METHODS);

    if (methodsIndex != -1 && methodsBegin != -1 && methodsEnd != -1) {
        frag.replace(methodsIndex, HIFI_UPDATE_METHODS.length(),
                        _shaderSource.mid(methodsBegin, methodsEnd - methodsBegin).toStdString());
    } else {
        success = false;
    }

    auto vertShader = gpu::Shader::createVertex(std::string(DrawUnitQuadTexcoord_vert));
    auto fragShader = gpu::Shader::createPixel(frag);

    auto program = gpu::Shader::createProgram(vertShader, fragShader);

    gpu::Shader::BindingSet slotBindings;
    gpu::Shader::makeProgram(*program, slotBindings);

    UPDATE_PARTICLES_BUFFER = program->getBuffers().findLocation(particleBuffer);
    UPDATE_HIFI_BUFFER = program->getBuffers().findLocation(hifiBuffer);
    int hifiBufferSize = program->getBuffers().findSlot(hifiBuffer)._size;
    _hifiBufferData.resize(hifiBufferSize / sizeof(float));
    _hifiBuffer = std::make_shared<Buffer>(hifiBufferSize, (const gpu::Byte*) &_hifiBufferData[0]);
    UPDATE_PARTICLES = program->getTextures().findLocation(particlesTex);

    if (UPDATE_PARTICLES_BUFFER == -1 || UPDATE_HIFI_BUFFER == -1 || UPDATE_PARTICLES == -1) {
        success = false;
    }

    _updatePipeline = gpu::Pipeline::create(program, state);

    // Drawing pipelines
    std::string vert = draw_procedural_particle_vert;

    int drawMethodsIndex = vert.find(HIFI_DRAW_METHODS);
    int drawMethodsBegin = _shaderSource.indexOf(BEGIN_HIFI_DRAW_METHODS) + BEGIN_HIFI_DRAW_METHODS.length();
    int drawMethodsEnd = _shaderSource.indexOf(END_HIFI_DRAW_METHODS);

    if (drawMethodsIndex != -1 && drawMethodsBegin != -1 && drawMethodsEnd != -1) {
        vert.replace(drawMethodsIndex, HIFI_DRAW_METHODS.length(),
                     _shaderSource.mid(drawMethodsBegin, drawMethodsEnd - drawMethodsBegin).toStdString());
    } else {
        success = false;
    }

    auto drawState = std::make_shared<gpu::State>();
    drawState->setCullMode(gpu::State::CULL_BACK);
    drawState->setDepthTest(true, false, gpu::LESS_EQUAL);
    drawState->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE,
                                gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);

    // Untextured pipeline
    auto notexVertShader = gpu::Shader::createVertex(vert);
    auto notexFragShader = gpu::Shader::createPixel(std::string(untextured_procedural_particle_frag));
    auto notexProgram = gpu::Shader::createProgram(notexVertShader, notexFragShader);

    gpu::Shader::BindingSet notexSlotBindings;
    gpu::Shader::makeProgram(*notexProgram, notexSlotBindings);

    NOTEX_DRAW_BUFFER = notexProgram->getBuffers().findLocation(particleBuffer);
    NOTEX_DRAW_PARTICLES = notexProgram->getTextures().findLocation(particlesTex);

    if (NOTEX_DRAW_BUFFER == -1 || NOTEX_DRAW_PARTICLES == -1) {
        success = false;
    }

    _untexturedPipeline = gpu::Pipeline::create(notexProgram, drawState);

    // Textured pipeline
    auto texVertShader = gpu::Shader::createVertex(vert);
    auto texFragShader = gpu::Shader::createPixel(std::string(textured_procedural_particle_frag));
    auto texProgram = gpu::Shader::createProgram(texVertShader, texFragShader);

    // Request these specifically because it doesn't work otherwise
    gpu::Shader::BindingSet texSlotBindings;
    texSlotBindings.insert(gpu::Shader::Binding(particlesTex, DRAW_PARTICLES));
    texSlotBindings.insert(gpu::Shader::Binding(colorMap, DRAW_TEXTURE));
    gpu::Shader::makeProgram(*texProgram, texSlotBindings);

    DRAW_BUFFER = texProgram->getBuffers().findLocation(particleBuffer);
    DRAW_PARTICLES = texProgram->getTextures().findLocation(particlesTex);
    DRAW_TEXTURE = texProgram->getTextures().findLocation(colorMap);

    if (DRAW_BUFFER == -1 || DRAW_PARTICLES == -1 || DRAW_TEXTURE == -1) {
        success = false;
    }

    _texturedPipeline = gpu::Pipeline::create(texProgram, drawState);

    _shadersCompiled = success;
}