#include "Window.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/CudaLbm.h"
#include "../Shizuku.Core/Ogl/Ogl.h"
#include "../Shizuku.Core/Ogl/Shader.h"

#include <GLFW/glfw3.h>
#include <GLUT/freeglut.h>
#include <typeinfo>
#include <memory>


namespace
{
    void ResizeWrapper(GLFWwindow* window, int width, int height)
    {
        Window::Instance().Resize(Rect<int>(width, height));
    }

    void MouseButtonWrapper(GLFWwindow* window, int button, int state, int mods)
    {
        Window::Instance().GlfwMouseButton(button, state, mods);
    }

    void MouseMotionWrapper(const int x, const int y)
    {
        Window::Instance().MouseMotion(x, y);
    }

    void MouseMotionWrapper(GLFWwindow* window, double x, double y)
    {
        Window::Instance().MouseMotion(x, y);
    }

    void MouseWheelWrapper(GLFWwindow* window, double xwheel, double ywheel)
    {
        Window::Instance().GlfwMouseWheel(xwheel, ywheel);
    }

    void KeyboardWrapper(const unsigned char key, const int x, const int y)
    {
        Window::Instance().Keyboard(key, x, y);
    }

    void GLAPIENTRY MessageCallback( GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam )
    {
      printf( "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
               ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
                type, severity, message );
    }



}

Window::Window() : 
    m_zoom(Zoom(*m_graphics)),
    m_pan(Pan(*m_graphics)),
    m_rotate(Rotate(*m_graphics)),
    m_addObstruction(AddObstruction(*m_graphics)),
    m_removeObstruction(RemoveObstruction(*m_graphics)),
    m_moveObstruction(MoveObstruction(*m_graphics)),
    m_pauseSimulation(PauseSimulation(*m_graphics))
{
}

void Window::SetGraphicsManager(GraphicsManager &graphics)
{
    m_graphics = &graphics;
}

void Window::RegisterCommands()
{
    m_zoom = Zoom(*m_graphics);
    m_pan = Pan(*m_graphics);
    m_rotate = Rotate(*m_graphics);
    m_addObstruction = AddObstruction(*m_graphics);
    m_removeObstruction = RemoveObstruction(*m_graphics);
    m_moveObstruction = MoveObstruction(*m_graphics);
    m_pauseSimulation = PauseSimulation(*m_graphics);
}

float Window::GetFloatCoordX(const int x)
{
    return static_cast<float>(x)/m_graphics->GetViewport().Width*2.f - 1.f;
}
float Window::GetFloatCoordY(const int y)
{
    return static_cast<float>(y)/m_graphics->GetViewport().Height*2.f - 1.f;
}

void Window::InitializeGL()
{
    glEnable(GL_LIGHT0);
    glewInit();
    glViewport(0,0,m_size.Width,m_size.Height);
}

void TimerEvent(int value)
{
    if (glutGetWindow())
    {
        glutPostRedisplay();
        glutTimerFunc(REFRESH_DELAY, TimerEvent, 0);
    }
}

void Window::GlfwResize(GLFWwindow* window, int width, int height)
{
    Resize(Rect<int>(width, height));
}

void Window::Resize(Rect<int> size)
{
    m_size = size;
    m_graphics->SetViewport(size);
}

void Window::GlfwMouseButton(const int button, const int state, const int mod)
{
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);

    float xf = GetFloatCoordX(x);
    float yf = GetFloatCoordY(m_size.Height - y);
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && state == GLFW_PRESS
        && mod == GLFW_MOD_CONTROL)
    {
        m_pan.Start(xf, yf);
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE && state == GLFW_PRESS)
    {
        m_rotate.Start(xf, yf);
        m_removeObstruction.Start(xf, yf);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && state == GLFW_PRESS)
    {
        m_addObstruction.Start(xf, yf);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && state == GLFW_PRESS)
    {
        m_moveObstruction.Start(xf, yf);
    }
    else
    {
        m_pan.End();
        m_rotate.End();
        m_moveObstruction.End();
        m_removeObstruction.End(xf, yf);
    }
}

void Window::MouseMotion(const int x, const int y)
{
    float xf = GetFloatCoordX(x);
    float yf = GetFloatCoordY(m_size.Height - y);
    m_pan.Track(xf, yf);
    m_rotate.Track(xf, yf);
    m_moveObstruction.Track(xf, yf);
}

void Window::Keyboard(const unsigned char key,
    const int x, const int y)
{
    if (key == 32)
    {
        if (m_graphics->GetCudaLbm()->IsPaused())
        {
            m_pauseSimulation.End();
        }
        else
        {
            m_pauseSimulation.Start();
        }
    }
}

void Window::GlfwMouseWheel(double xwheel, double ywheel)
{
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    const int dir = ywheel > 0 ? 1 : 0;
    m_zoom.Start(dir, 0.3f);
}

void Window::GlfwUpdateWindowTitle(const float fps, const Rect<int> &domainSize, const int tSteps)
{
    char fpsReport[256];
    int xDim = domainSize.Width;
    int yDim = domainSize.Height;
    sprintf_s(fpsReport, 
        "Interactive CFD running at: %i timesteps/frame at %3.1f fps = %3.1f timesteps/second on %ix%i mesh",
        tSteps, fps, TIMESTEPS_PER_FRAME*fps, xDim, yDim);
    glfwSetWindowTitle(m_window, fpsReport);
}

void Window::GlfwDrawLoop()
{
    m_fpsTracker.Tick();
    GraphicsManager* graphicsManager = m_graphics;
    graphicsManager->UpdateGraphicsInputs();
    graphicsManager->GetCudaLbm()->UpdateDeviceImage();

    graphicsManager->RunSimulation();

    // render caustic floor to texture
    graphicsManager->RenderFloorToTexture();

    graphicsManager->RunSurfaceRefraction();

    //int viewX, viewY;
    //m_graphics->GetViewport(viewX, viewY);
    //ResizeWrapper(m_window, viewX, viewY);

    graphicsManager->UpdateViewMatrices();
    graphicsManager->UpdateViewTransformations();


    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    graphicsManager->RenderVbo();

    //Layout::Draw2D(*m_windowPanel);


    m_fpsTracker.Tock();

    CudaLbm* cudaLbm = graphicsManager->GetCudaLbm();
    const int tStepsPerFrame = graphicsManager->GetCudaLbm()->GetTimeStepsPerFrame();
    GlfwUpdateWindowTitle(m_fpsTracker.GetFps(), cudaLbm->GetDomainSize(), tStepsPerFrame);
}

void Window::InitializeGlfw()
{
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(m_size.Width, m_size.Height, "LearnOpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    m_window = window;

    // Set the required callback functions
    //glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, ResizeWrapper); //glfw is in C so cannot bind instance method...
    glfwSetCursorPosCallback(window, MouseMotionWrapper);
    glfwSetMouseButtonCallback(window, MouseButtonWrapper);
    glfwSetScrollCallback(window, MouseWheelWrapper);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback(MessageCallback, 0);
}

void Window::GlfwDisplay()
{
//    Ogl ogl;
//    ////Compile shader programs
//    std::shared_ptr<ShaderProgram> basicShader = ogl.CreateShaderProgram("Basic");;
//    basicShader->CreateShader("basic.vert.glsl", GL_VERTEX_SHADER);
//    basicShader->CreateShader("basic.frag.glsl", GL_FRAGMENT_SHADER);

//    GLfloat vertices[] = {
//        0.0f, 0.0f, 0.5f,
//        0.0f, 0.5f, 0.5f,
//        0.5f, 0.5f, 0.5f
//    };
//    unsigned int indices[] = {
//        0, 1, 2
//    };

//    std::shared_ptr<Ogl::Vao> vao = ogl.CreateVao("Vao");
//    vao->Bind();

//    //Sphere buffer
//    std::shared_ptr<Ogl::Buffer> vbo = ogl.CreateBuffer(GL_ARRAY_BUFFER, vertices, 3*3, "triangle", GL_DYNAMIC_DRAW);
//    ogl.BindBO(GL_ARRAY_BUFFER, *vbo);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

//    std::shared_ptr<Ogl::Buffer> ebo = ogl.CreateBuffer(GL_ELEMENT_ARRAY_BUFFER, indices, 1*3, "triangle", GL_DYNAMIC_DRAW);
//    ogl.UnbindBO(GL_ARRAY_BUFFER);

//    vao->Unbind();


//    glm::mat4 model;
//    glm::mat4 view;
//    glm::mat4 projection;
//    glm::vec3 cameraPos{ 0.f, 0.0f, 2.0f };
//    glm::vec3 cameraFront{ 0.f, 0.0f, -1.0f };
//    glm::vec3 cameraUp{ 0.f, 1.0f, 0.0f };


    while (!glfwWindowShouldClose(m_window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        GlfwDrawLoop();

//        vao->Bind();

//        // Render
//        // Clear the colorbuffer
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



//        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 10.0f);





//        //Draw ref frame
//        basicShader->Use();
//        ogl.BindBO(GL_ARRAY_BUFFER, *vbo);
//        ogl.BindBO(GL_ELEMENT_ARRAY_BUFFER, *ebo);

//        basicShader->SetUniform("Transform", view);
//        basicShader->SetUniform("Projection", projection);
// 
//        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (const void*)(0));

//        ogl.UnbindBO(GL_ARRAY_BUFFER);
//        ogl.UnbindBO(GL_ELEMENT_ARRAY_BUFFER);
//        basicShader->Unset();




//        vao->Unbind();

        glfwSwapBuffers(m_window);
    }
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    //glutMainLoop();
}


