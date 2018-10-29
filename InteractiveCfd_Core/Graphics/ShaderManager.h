#pragma once
#include "common.h"
#include "cuda_runtime.h"
#include <GLEW/glew.h>
#include "cuda_gl_interop.h"  // needs GLEW
#include <glm/glm.hpp>
#include <vector>
#include <string>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifdef LBM_GL_CPP_EXPORTS  
#define FW_API __declspec(dllexport)   
#else  
#define FW_API __declspec(dllimport)   
#endif  

class CudaLbm;
class ShaderProgram;
class Domain;

class FW_API ShaderManager
{
private:
    class Ssbo
    {
    public:
        GLuint m_id;
        std::string m_name;
    };
    CudaLbm* m_cudaLbm;
    cudaGraphicsResource* m_cudaGraphicsResource;
    cudaGraphicsResource* m_cudaFloorLightTextureResource;
    cudaGraphicsResource* m_cudaEnvTextureResource;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_elementArrayBuffer;
    GLuint m_floorLightTexture;
    GLuint m_envTexture;
    GLuint m_floorFbo;
    ShaderProgram* m_shaderProgram;
    ShaderProgram* m_lightingProgram;
    ShaderProgram* m_obstProgram;
    ShaderProgram* m_floorProgram;
    std::vector<Ssbo> m_ssbos;
    float m_omega;
    float m_inletVelocity;
public:
    ShaderManager();

    void CreateCudaLbm();
    CudaLbm* GetCudaLbm();
    cudaGraphicsResource* GetCudaSolutionGraphicsResource();
    cudaGraphicsResource* GetCudaFloorLightTextureResource();
    cudaGraphicsResource* GetCudaEnvTextureResource();
    GLuint GetVbo();
    GLuint GetElementArrayBuffer();
    void CreateVbo(const unsigned int size, const unsigned int vboResFlags);
    void DeleteVbo();
    void CreateElementArrayBuffer();
    void DeleteElementArrayBuffer();
    template <typename T> void CreateShaderStorageBuffer(T defaultValue,
        const unsigned int sizeInInts, const std::string name);
    GLuint GetShaderStorageBuffer(const std::string name);
    void CreateVboForCudaInterop(unsigned int size);
    void CleanUpGLInterOp();
    ShaderProgram* GetShaderProgram();
    ShaderProgram* GetLightingProgram();
    ShaderProgram* GetObstProgram();
    ShaderProgram* GetFloorProgram();
    void CompileShaders();
    void AllocateStorageBuffers();
    void SetUpTextures();
    void InitializeObstSsbo();
    void InitializeComputeShaderData();

    void BindFloorLightTexture();
    void BindEnvTexture();
    void UnbindFloorTexture();
    
    void SetOmega(const float omega);
    float GetOmega();
    void SetInletVelocity(const float u);
    float GetInletVelocity();
    void UpdateLbmInputs(const float u, const float omega);
    void RunComputeShader(const glm::vec3 cameraPosition, const ContourVariable contVar,
        const float contMin, const float contMax);
    void UpdateObstructionsUsingComputeShader(const int obstId, Obstruction &newObst, const float scaleFactor);
    int RayCastMouseClick(glm::vec3 &rayCastIntersection, const glm::vec3 rayOrigin,
        const glm::vec3 rayDir);
    void RenderFloorToTexture(Domain &domain);
    void RenderVbo(const bool renderFloor, Domain &domain, const glm::mat4 &modelMatrix,
        const glm::mat4 &projectionMatrix);
    void RenderVboUsingShaders(const bool renderFloor, Domain &domain,
        const glm::mat4 &modelMatrix, const glm::mat4 &projectionMatrix);
};

void SetUniform(GLuint shaderId, const GLchar* varName, const int varValue);
void SetUniform(GLuint shaderId, const GLchar* varName, const float varValue);
void SetUniform(GLuint shaderId, const GLchar* varName, const bool varValue);
void SetUniform(GLuint shaderId, const GLchar* varName, const glm::vec3 varValue);
void RunSubroutine(GLuint shaderId, const GLchar* subroutineName,
    const int3 workGroupSize);
