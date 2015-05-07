#define _USE_MATH_DEFINES
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <utility>
#include <iostream>
#include <string>
#include <stdexcept>
#include <glew/glew.h>
#include <GLFW/glfw3.h>
#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

#include "GLSL.h"
#include "GLUtils.h"
#include "GLWorldState.h"
#include "SceneContext.h"
#include "Config.h"
#include "Camera.h"
#include "Options.h"
#include "Raytrace.h"

/******************************************************************************/

using namespace glm;
using namespace std;
using namespace cimg_library;

/******************************************************************************/

static Camera rayTraceCamera;
static shared_ptr<Image> output;

// Attributes
static GLint locationPos;
static GLint locationCol;
static GLint locationNor;

// Uniforms
static GLint unifModel;
static GLint unifModelInvTr;
static GLint unifViewProj;
static GLint unifEyePos;
static GLint unifLightPos;
static GLint unifLightColor;

// Shader programs:
static GLuint shaderProgram;

/*******************************************************************************
 * Vertex shader
 ******************************************************************************/
const GLchar* vertexShader = GLSL(150,

uniform vec4 u_EyePos;
uniform mat4 u_Model;
uniform mat4 u_ModelInvTr;
uniform mat4 u_ViewProj;
uniform vec4 u_LightPos;

in vec3 vs_Position;
in vec3 vs_Normal;
in vec3 vs_Color;

out vec4 fs_V;
out vec4 fs_Normal;
out vec4 fs_LightVector;
out vec4 fs_Color;

void main()
{
    fs_Color  = vec4(vs_Color, 1);
    fs_Normal = u_ModelInvTr * vec4(vs_Normal, 0);

    vec4 modelPosition = u_Model * vec4(vs_Position, 1);

    // Set up our vector for the light
    fs_LightVector = u_LightPos - modelPosition;

    // Built-in things to pass down the pipeline
    gl_Position = u_ViewProj * modelPosition;

    // Eye-to-vertex direction
    fs_V = modelPosition - u_EyePos;
}
);

/*******************************************************************************
 * Fragment shader
 ******************************************************************************/

const GLchar* fragmentShader = GLSL(150,

uniform vec4 u_LightColor;

in vec4 fs_V;
in vec4 fs_Normal;
in vec4 fs_LightVector;
in vec4 fs_Color;

out vec4 out_Color;

void main()
{
    vec4 matColor = fs_Color;

    vec4 N = normalize(fs_Normal);
    vec4 L = normalize(fs_LightVector);
    vec4 V = normalize(fs_V);

    float diffuse  = 0.75;
    float specular = 1.0;

    float highlightSize = 32;
    float intensity     = 10;

    float Ia = 0.2;
    float Id = max(0, dot(N, L));
    float Is = pow(max(0, dot(normalize(reflect(L, N)), V)), highlightSize);

    vec3 Ka  = Ia * matColor.rgb;
    vec3 Kd  = diffuse * Id * matColor.rgb * u_LightColor.rgb;
    vec3 Ks  = specular * Is * intensity * u_LightColor.rgb;

    out_Color = vec4(Ka + Kd + Ks, matColor.a);
}
);

// Scene state & render options ////////////////////////////////////////////////

static shared_ptr<GLWorldState> state(nullptr);
static shared_ptr<SceneContext> sceneContext(nullptr);
static shared_ptr<TraceOptions> traceOptions(nullptr);

// Animation/transformation stuff //////////////////////////////////////////////

clock_t old_time;
float rotation = 0.0f;

// OpenGL drawing functions ////////////////////////////////////////////////////

static void initShader();
static void cleanupShader();
static void uploadGeometry();
static void cleanup();
static void finish();

static void initGLPreviewWindow(int argc, char** argv, const std::string& title);
static void display();
static void handleWindowResize(GLFWwindow* window, int width, int height);
static void handleError(int error, const char* description);
static void handleKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods);

/**
 * Initiates actual raytracing
 */
static void runRaytracer(bool disablePreview = false)
{
    assert((!!sceneContext) && (!!traceOptions));

    rayTrace(output, rayTraceCamera, sceneContext, traceOptions);

    if (!traceOptions->enablePixelDebug) {

        string outputFile = Utils::cwd("output.png");
        output->save(outputFile.c_str());
        cout << "Output written to " << outputFile << endl;
    
    } else {

        if (disablePreview) {
            finish();
        }
    }
}

/**
 * Dump the configuration settings to stdout and quit
 */
static void printConfigAndQuit(shared_ptr<Configuration>& config)
{
    LOG(INFO) << config << endl;
    exit(EXIT_SUCCESS);
}

/**
 * Main
 */
int main(int argc, char** argv)
{
    // Adapted from "The Lean Mean Option Parser" documentation at
    // http://optionparser.sourceforge.net/index.html

    // Skip program name argv[0] if present:
    argc -= argc > 0;
    argv += argc > 0;

    shared_ptr<Configuration> config(nullptr);
    vec2 resolution;
    option::Stats  stats(usage, argc, argv);

    option::Option* options = new option::Option[stats.options_max];
    option::Option* buffer  = new option::Option[stats.buffer_max];

    option::Parser parse(usage, argc, argv, options, buffer);

    if (parse.error() || options[HELP] || argc == 0) {
        if (parse.error()) {
            LOG(ERROR) << "[!] option parse error" << endl;
        }
        option::printUsage(std::cout, usage);
        goto failure;
    }

    // Seed PRNG
    srand(static_cast<unsigned int>(time(nullptr)));

    // Parse configuration
    config = make_shared<Configuration>(argv[argc-1]);
    try {

        sceneContext = move(config->read());

    } catch (std::runtime_error& e) {

        LOG(ERROR) << "[!] Configuration reader error: " << e.what() << endl;
        goto failure;
    }

    // Dump configuration to stdout
    if (options[PRINT_CONFIG_AND_QUIT]) {
        printConfigAndQuit(config);
    }

    // Create the world state object from the current configuration:
    state = unique_ptr<GLWorldState>(new GLWorldState(sceneContext->getSceneGraph()));

    // Configure options used during ray tracing
    traceOptions = shared_ptr<TraceOptions>(new TraceOptions());

    // Samples per pixel:
    if (options[SAMPLES_PER_PIXEL].count() > 0) {
        auto str = options[SAMPLES_PER_PIXEL].first()->arg;
        if (str) {
            traceOptions->samplesPerPixel = Utils::parseNumber(string(str), TraceOptions::SAMPLES_PER_PIXEL_DEFAULT);
        } else {
            traceOptions->samplesPerPixel = TraceOptions::SAMPLES_PER_PIXEL_DEFAULT;
        }
    }

    // Samples per light:
    if (options[SAMPLES_PER_LIGHT].count() > 0) {
        auto str = options[SAMPLES_PER_LIGHT].first()->arg;
        if (str) {
            traceOptions->samplesPerLight = Utils::parseNumber(string(str), TraceOptions::SAMPLES_PER_LIGHT_DEFAULT);
        } else {
            traceOptions->samplesPerLight = TraceOptions::SAMPLES_PER_LIGHT_DEFAULT;
        }
    }

    // Was a debug pixel specified?
    if (options[DEBUG_PIXEL].count() >= 2) {

        int px = Utils::parseNumber(options[DEBUG_PIXEL].first()->arg, -1);
        int py = Utils::parseNumber(options[DEBUG_PIXEL].next()->arg, -1);

        if (px != -1 && py != -1) {

            cout << endl
                 << "********************************************************************************" << endl
                 << "DEBUGGING ON PIXEL ("<< px <<"," << py << ")" << endl
                 << "********************************************************************************" << endl
                 << endl;

            traceOptions->enablePixelDebug = true;
            traceOptions->xDebugPixel      = px;
            traceOptions->yDebugPixel      = py;
        }
    }

    // Initialize raytracer code:
    resolution = sceneContext->getResolution();
    initRaytrace(rayTraceCamera, sceneContext);

    // Dimension the output image:
    output = shared_ptr<CImg<unsigned char>>(make_shared<CImg<unsigned char>>(resolution.x, resolution.y, 1, 3, 0));

    if (options[PRINT_CAMERA]) {
        LOG(INFO) << rayTraceCamera << endl;
    }

    // If no preview, rendering starts immediately:
    if (options[DISABLE_PREVIEW]) {
        runRaytracer(true);
    } else {
        initGLPreviewWindow(argc, argv, "raycpp :: OpenGL Preview");
    }

    exit(EXIT_SUCCESS);

failure:
    
    delete[] options;
    delete[] buffer;

    exit(EXIT_FAILURE);
}

/**
 * Initializes the OpenGL preview window
 */
void initGLPreviewWindow(int argc, char** argv, const string& title)
{
    GLFWwindow* window = nullptr;

    #ifdef DEBUG
    LOG(DEBUG) << "- initGLPreviewWindow()" << endl;
    #endif

    if (!glfwInit()) {
        LOG(ERROR) << "glfwInit() failed" << endl;
        exit(EXIT_FAILURE);
    }

    // Force at least OpenGL 3.2 on Mac by using the "Core" profile:
    #if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    int windowWidth  = sceneContext->getResolution().x;
    int windowHeight = sceneContext->getResolution().y;

    window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        LOG(ERROR) << "glfwCreateWindow() failed" << endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        LOG(ERROR) << "glewInit() failed" << endl;
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(handleError);
    glfwSetKeyCallback(window, handleKeyPress);
    glfwSetWindowSizeCallback(window, handleWindowResize);

    // More init stuff:
    // Set the color which clears the screen between frames
    glClearColor(0, 0, 0, 1);

    // Enable and clear the depth buffer
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);

    // Initialize the shader:
    initShader();

    // Send the geometry data to the GPU:
    uploadGeometry();

    handleWindowResize(window, windowWidth, windowHeight);

    old_time = clock();

    while (!glfwWindowShouldClose(window)) {

        display();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

/**
 *
 */
void cleanup()
{
    #ifdef DEBUG
    LOG(DEBUG) << "- cleanup()" << endl;
    #endif

    glDeleteProgram(shaderProgram);
}

/**
 *
 */
void finish()
{
    cleanup();
    exit(EXIT_SUCCESS);
}

/**
 * Initializes the shader
 */
void initShader()
{
    LOG(DEBUG) << "- initShader()" << endl;

    // Tell the GPU to create new shaders and a shader program
    GLuint shadVert = glCreateShader(GL_VERTEX_SHADER);
    GLuint shadFrag = glCreateShader(GL_FRAGMENT_SHADER);
    shaderProgram   = glCreateProgram();

    // Load and compile each shader program and check to make sure 
    // the shaders complied correctly

    // - Vertex shader
    glShaderSource(shadVert, 1, &vertexShader, nullptr);
    glCompileShader(shadVert);
    GLUtils::printShaderInfoLog(shadVert);

    // - Diffuse fragment shader
    glShaderSource(shadFrag, 1, &fragmentShader, nullptr);
    glCompileShader(shadFrag);
    GLUtils::printShaderInfoLog(shadFrag);

    // Link the shader programs together from compiled bits
    glAttachShader(shaderProgram, shadVert);
    glAttachShader(shaderProgram, shadFrag);
    glBindFragDataLocation(shaderProgram, 0, "out_Color");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    GLUtils::printLinkInfoLog(shaderProgram);

    // Clean up the shaders now that they are linked
    glDetachShader(shaderProgram, shadVert);
    glDetachShader(shaderProgram, shadFrag);
    glDeleteShader(shadVert);
    glDeleteShader(shadFrag);

    // Find out what the GLSL locations are, since we can't pre-define these
    locationPos    = glGetAttribLocation(shaderProgram, "vs_Position");
    locationNor    = glGetAttribLocation(shaderProgram, "vs_Normal");
    locationCol    = glGetAttribLocation(shaderProgram, "vs_Color");
    unifViewProj   = glGetUniformLocation(shaderProgram, "u_ViewProj");
    unifModel      = glGetUniformLocation(shaderProgram, "u_Model");
    unifModelInvTr = glGetUniformLocation(shaderProgram, "u_ModelInvTr");
    unifEyePos     = glGetUniformLocation(shaderProgram, "u_EyePos");
    unifLightPos   = glGetUniformLocation(shaderProgram, "u_LightPos");
    unifLightColor = glGetUniformLocation(shaderProgram, "u_LightColor");

    GLUtils::printErrorLog();
}

/**
 * Uploads the geometry contained in the given scene graph node
 */
static void* uploadNode(shared_ptr<GraphNode> node, void* ignore, int depth)
{
    LOG(DEBUG) << "-- uploadNode()" << endl;

    shared_ptr<GLGeometry> instance = node->getInstance();

    if (instance) {
        instance->upload(shaderProgram, locationPos, locationNor, locationCol);
    }

    return nullptr;
}

/**
 * Walks the scene graph, uploading the geometry contained in the graph's nodes
 */
void uploadGeometry()
{
    LOG(DEBUG) << "- uploadGeometry()" << endl;

    auto root = sceneContext->getSceneGraph().getRoot();

    if (root) {
        walk(root, uploadNode, static_cast<void*>(nullptr));
    }
}

/** 
 * Transforms the given affine matrix current according to the geometries
 * position in the scene graph, then draws tha actual geometry using the
 * updated affine matrix
 */
static mat4 drawGLGeometry(shared_ptr<GraphNode> node, mat4 current, int depth)
{
    mat4 next = applyTransform(node, current);

    if (state->doRotateScene() && node->isRoot()) {
        next *= rotate(mat4(), rotation, vec3(0.0, 1.0f, 0.0f));
    }

    shared_ptr<GLGeometry> instance = node->getInstance();

    if (instance) {
        instance->draw(*state, shaderProgram, unifModel, unifModelInvTr, next);
    }

    return next;
}

/**
 * Main display function
 */
void display()
{
    clock_t newTime = clock();
    rotation += 50.0f * (static_cast<float>(newTime - old_time) / static_cast<float>(CLOCKS_PER_SEC));
    old_time = newTime;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- Export the uniforms -------------------------------------------------

    // Eye position
    vec3 eyePosition = sceneContext->getEyePosition();
    glUniform4f(unifEyePos, x(eyePosition), y(eyePosition), z(eyePosition), 1.0f);

    // Eye position
    PointLight light = state->getPreviewLight();

    // Position
    P lpos = light.getPosition();
    glUniform4f(unifLightPos, x(lpos), y(lpos), z(lpos), 1.0f);

    // Color
    Color lcol = light.getColor();
    glUniform4f(unifLightColor, lcol.fR(), lcol.fG(), lcol.fB(), 1.0f);

    if (state->doLightHueChange()) {
        state->shiftGlobalLightHue();
    }

    // --- Render the scene ----------------------------------------------------

    auto root = sceneContext->getSceneGraph().getRoot();

    if (root) {
        walk(root, drawGLGeometry, mat4());
    }

    // -------------------------------------------------------------------------

    GLUtils::printErrorLog();
}

/**
 * Handler for window resizing
 */
void handleWindowResize(GLFWwindow* window, int width, int height)
{
    LOG(DEBUG) << "- handleWindowResize() = <" << width << "," << height << ">" << endl;

    glViewport(0, 0, width, height);

    mat4 projection = perspective(radians(sceneContext->getFOVAngle())
                                           ,sceneContext->getAspectRatio()
                                           ,sceneContext->getZNear()
                                           ,sceneContext->getZFar());

    mat4 camera = lookAt(sceneContext->getEyePosition()
                                  ,sceneContext->getLookAtPosition()
                                  ,sceneContext->getUpDir());

    projection *= camera;

    // Must specify which shader program the matrix is to be used in first:
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(unifViewProj, 1, GL_FALSE, &projection[0][0]);
}

/**
 * Handler for errors
 */
void handleError(int error, const char* description)
{
    LOG(ERROR) << "Error: " << description << endl;
}

/**
 * Handler for key presses
 */
void handleKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    LOG(DEBUG) << "- handleKeyPress() = '" << key << "'" << endl;

    switch (key) {
        case GLFW_KEY_P:
            {
                if (action == GLFW_PRESS) {
                    runRaytracer(true);
                }
            }
            break;
        case GLFW_KEY_SPACE :
            {
                if (action == GLFW_PRESS) {
                    state->toggleRotateScene();
                }
            }
            break;
        case GLFW_KEY_N:
            {
                if (action == GLFW_PRESS) {
                    state->highlightNextNode();
                }
            }
            break;
        case GLFW_KEY_A:
            {
                if (action == GLFW_PRESS) {
                    state->translateSelectedXNeg();
                }
            }
            break;
        case GLFW_KEY_D:
            {
                if (action == GLFW_PRESS) {
                    state->translateSelectedXPos();
                }
            }
            break;
        case GLFW_KEY_S:
            {
                if (action == GLFW_PRESS) {
                    state->translateSelectedYNeg();
                }
            }
            break;
        case GLFW_KEY_W:
            {
                if (action == GLFW_PRESS) {
                    state->translateSelectedYPos();
                }
            }
            break;
        case GLFW_KEY_R:
            {
                if (action == GLFW_PRESS) {
                    state->translateSelectedZNeg();
                }
            }
            break;
        case GLFW_KEY_E:
            {
                if (action == GLFW_PRESS) {
                    state->translateSelectedZPos();
                }
            }
            break;
        case GLFW_KEY_X:
            {
                if (action == GLFW_PRESS) {
                    if (mods & GLFW_MOD_SHIFT) {
                        state->scaleDecreaseSelectedX();
                    } else {
                        state->scaleIncreaseSelectedX();
                    }
                }
            }
            break;
        case GLFW_KEY_Y:
            {
                if (action == GLFW_PRESS) {
                    if (mods & GLFW_MOD_SHIFT) {
                        state->scaleDecreaseSelectedY();
                    } else {
                        state->scaleIncreaseSelectedY();
                    }
                }
            }
            break;
        case GLFW_KEY_Z:
            {
                if (action == GLFW_PRESS) {
                    if (mods & GLFW_MOD_SHIFT) {
                        state->scaleDecreaseSelectedZ();
                    } else {
                        state->scaleIncreaseSelectedZ();
                    }
                }
            }
            break;
        case GLFW_KEY_J:
            {
                if (action == GLFW_PRESS) {
                    if (mods & GLFW_MOD_SHIFT) {
                        state->rotateSelectedNegX();
                    } else {
                        state->rotateSelectedPosX();
                    }
                }
            }
            break;
        case GLFW_KEY_K:
            {
                if (action == GLFW_PRESS) {
                    if (mods & GLFW_MOD_SHIFT) {
                        state->rotateSelectedNegY();
                    } else {
                        state->rotateSelectedPosY();
                    }
                }
            }
            break;

        case GLFW_KEY_L:
            {
                if (action == GLFW_PRESS) {
                    if (mods & GLFW_MOD_SHIFT) {
                        state->rotateSelectedNegZ();
                    } else {
                        state->rotateSelectedPosZ();
                    }
                }
            }
            break;
        case GLFW_KEY_SLASH:
            {
                if (action == GLFW_PRESS) {
                    state->switchPolygonMode();
                }
            }
            break;
        case GLFW_KEY_PERIOD:
            {
                if (action == GLFW_PRESS) {
                    if (state->deleteSelectedNode()) {
                        finish();
                    }
                }
            }
            break;
        case GLFW_KEY_F:
            {
                if (action == GLFW_PRESS) {
                    if (mods & GLFW_MOD_SHIFT) {
                        state->translateLightNegX();
                    } else {
                        state->translateLightPosX();
                    }
                }
            }
            break;
        case GLFW_KEY_G:
            {
                if (mods & GLFW_MOD_SHIFT) {
                    state->translateLightNegY();
                } else {
                    state->translateLightPosY();
                }
            }
            break;
        case GLFW_KEY_H:
            {
                if (action == GLFW_PRESS) {
                    if (mods & GLFW_MOD_SHIFT) {
                        state->translateLightNegZ();
                    } else {
                        state->translateLightPosZ();
                    }
                }
            }
            break;
        case GLFW_KEY_BACKSLASH:
            {
                if (action == GLFW_PRESS) {
                    state->toggleLightHueChange();
                }
            }
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            {
                if (action == GLFW_PRESS) {
                    state->shiftGlobalLightHue();
                }
            }
            break;
        case GLFW_KEY_Q:
            {
                if (action == GLFW_PRESS) {
                    finish();
                }
            }
            break;
    }
}
