#pragma once
#include "Shader.h"
#include "common.h"
#include "cuda_runtime.h"
#include "cuda_gl_interop.h"
#include "helper_cuda.h"
#include "helper_cuda_gl.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifdef LBM_GL_CPP_EXPORTS  
#define FW_API __declspec(dllexport)   
#else  
#define FW_API __declspec(dllimport)   
#endif  

class Panel;
class Mouse;
class Domain;

class FW_API CudaLbm
{
private:
    int m_maxX;
    int m_maxY;
    Domain* m_domain;
    float* m_fA_d;
    float* m_fB_d;
    int* m_Im_d;
    float* m_FloorTemp_d;
    Obstruction* m_obst_d;
    Obstruction m_obst_h[MAXOBSTS];
    float m_inletVelocity;
    float m_omega;
    bool m_isPaused;
    int m_timeStepsPerFrame;
public:
    CudaLbm();
    CudaLbm(const int maxX, const int maxY);
    Domain* GetDomain();
    float* GetFA();
    float* GetFB();
    int* GetImage();
    float* GetFloorTemp();
    Obstruction* GetDeviceObst();
    Obstruction* GetHostObst();
    float GetInletVelocity();
    float GetOmega();
    void SetInletVelocity(const float velocity);
    void SetOmega(const float omega);
    void SetPausedState(const bool isPaused);
    bool IsPaused();
    int GetTimeStepsPerFrame();
    void SetTimeStepsPerFrame(const int timeSteps);

    void AllocateDeviceMemory();
    void InitializeDeviceMemory();
    void DeallocateDeviceMemory();
    void UpdateDeviceImage();
    int ImageFcn(const int x, const int y);

   
};

class FW_API Graphics
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
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_elementArrayBuffer;
    ShaderProgram* m_shaderProgram;
    ShaderProgram* m_lightingProgram;
    ShaderProgram* m_obstProgram;
    std::vector<Ssbo> m_ssbos;
    float m_omega;
    float m_inletVelocity;
public:
    Graphics();

    void CreateCudaLbm();
    CudaLbm* GetCudaLbm();
    cudaGraphicsResource* GetCudaSolutionGraphicsResource();
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
    void CompileShaders();
    void AllocateStorageBuffers();
    void InitializeObstSsbo();
    void InitializeComputeShaderData();
    
    void SetOmega(const float omega);
    float GetOmega();
    void SetInletVelocity(const float u);
    float GetInletVelocity();
    void UpdateLbmInputs(const float u, const float omega);
    void RunComputeShader(const float3 cameraPosition);
    void UpdateObstructionsUsingComputeShader(const int obstId, Obstruction &newObst);
    int RayCastMouseClick(float3 &rayCastIntersection, const float3 rayOrigin,
        const float3 rayDir);
    void RenderVbo(const bool renderFloor, Domain &domain, const glm::mat4 &modelMatrix,
        const glm::mat4 &projectionMatrix);
    void RenderVboUsingShaders(const bool renderFloor, Domain &domain,
        const glm::mat4 &modelMatrix, const glm::mat4 &projectionMatrix);
};



class FW_API GraphicsManager
{
private:
    float m_currentZ = -1000.f;
    //view transformations
    float3 m_rotate;
    float3 m_translate;
    int m_currentObstId = -1;
    float m_currentObstSize = 0.f;
    Shape m_currentObstShape = Shape::SQUARE;
    ViewMode m_viewMode;
    Obstruction* m_obstructions;
    Panel* m_parent;
    float m_scaleFactor = 1.f;
    GLint m_viewport[4];
    GLdouble m_modelMatrix[16];
    GLdouble m_projectionMatrix[16];
    float m_contourMinValue;
    float m_contourMaxValue;
    ContourVariable m_contourVar;
    Graphics* m_graphics;
    bool m_useCuda = true;

public:
    float4* m_rayCastIntersect_d;

    GraphicsManager(Panel* panel);

    void UseCuda(bool useCuda);

    float3 GetRotationTransforms();
    float3 GetTranslationTransforms();

    void SetCurrentObstSize(const float size);

    Shape GetCurrentObstShape();
    void SetCurrentObstShape(const Shape shape);

    ViewMode GetViewMode();
    void SetViewMode(const ViewMode viewMode);

    float GetContourMinValue();
    float GetContourMaxValue();
    void SetContourMinValue(const float contourMinValue);
    void SetContourMaxValue(const float contourMaxValue);
    ContourVariable GetContourVar();
    void SetContourVar(const ContourVariable contourVar);

    void SetObstructionsPointer(Obstruction* obst);

    float GetScaleFactor();
    void SetScaleFactor(const float scaleFactor);

    CudaLbm* GetCudaLbm();
    Graphics* GetGraphics();

    void CenterGraphicsViewToGraphicsPanel(const int leftPanelWidth);
    void SetUpGLInterop();
    void SetUpShaders();
    void SetUpCuda();
    void RunCuda();
    void RunComputeShader();
    void RunSimulation();
    void RenderVbo();
    bool ShouldRenderFloor();

    void Zoom(const int dir, const float mag);
    void Pan(const float dx, const float dy);
    void Rotate(const float dx, const float dy);
    int PickObstruction(const float mouseXf, const float mouseYf);
    int PickObstruction(const int mouseX, const int mouseY);
    void UnpickObstruction();
    void MoveObstruction(int obstId, const float mouseXf, const float mouseYf,
        const float dxf, const float dyf);
   
    void UpdateViewTransformations();
    void UpdateGraphicsInputs();
    void UpdateLbmInputs();
    glm::vec4 GetViewportMatrix();
    glm::mat4 GetModelMatrix();
    glm::mat4 GetProjectionMatrix();

    void GetSimCoordFromMouseCoord(int &xOut, int &yOut, const int mouseX, const int mouseY);
    void GetSimCoordFromFloatCoord(int &xOut, int &yOut, const float xf, const float yf);
    void GetMouseRay(float3 &rayOrigin, float3 &rayDir, const int mouseX, const int mouseY);
    int GetSimCoordFrom3DMouseClickOnObstruction(int &xOut, int &yOut, 
        const int mouseX, const int mouseY);
    void GetSimCoordFromMouseRay(int &xOut, int &yOut, const int mouseX, const int mouseY);
    void GetSimCoordFromMouseRay(int &xOut, int &yOut, const float mouseXf, const float mouseYf,
        const float planeZ);
    void GetSimCoordFromMouseRay(int &xOut, int &yOut, const int mouseX, const int mouseY,
        const float planeZ);
    void AddObstruction(const int simX, const int simY);
    void RemoveObstruction(const int simX, const int simY);
    void RemoveSpecifiedObstruction(const int obstId);
    void MoveObstruction(const int xi, const int yi, const float dxf, const float dyf);
    int FindUnusedObstructionId();
    int FindClosestObstructionId(const int simX, const int simY);
    int FindObstructionPointIsInside(const int x, const int y, const float tolerance=0.f);
    bool IsInClosestObstruction(const int mouseX, const int mouseY);
 
};

float GetDistanceBetweenTwoPoints(const float x1, const float y1,
    const float x2, const float y2);
void SetUniform(GLuint shaderId, const GLchar* varName, const int varValue);
void SetUniform(GLuint shaderId, const GLchar* varName, const float varValue);
void SetUniform(GLuint shaderId, const GLchar* varName, const bool varValue);
void SetUniform(GLuint shaderId, const GLchar* varName, const float3 varValue);
void RunSubroutine(GLuint shaderId, const GLchar* subroutineName,
    const int3 workGroupSize);