#define _USE_MATH_DEFINES
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <time.h>
#include <glew/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "GLUtils.h"
#include "WorldState.h"
#include "Config.h"
#include "ObjReader.h"
#include "Camera.h"
#include "Raytrace.h"
#include "tests.h"

#include "optionparser.h"
#include <EasyBMP/EasyBMP.h>
//#include <chibi/eval.h>

// Namespace ///////////////////////////////////////////////////////////////////

using namespace std;

// Raytracer related ///////////////////////////////////////////////////////////

static Camera rayTraceCamera;
static BMP output;

// Attributes //////////////////////////////////////////////////////////////////

static GLint locationPos;
static GLint locationCol;
static GLint locationNor;

// Uniforms ////////////////////////////////////////////////////////////////////

static GLint unifModel;
static GLint unifModelInvTr;
static GLint unifViewProj;
static GLint unifEyePos;
static GLint unifLightPos;
static GLint unifLightColor;

// Shaders /////////////////////////////////////////////////////////////////////

static GLuint shaderProgram;

// -- Vertex shader ------------------------------------------------------------
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
    //fs_V = modelPosition - u_EyePos;
    fs_V = modelPosition - u_EyePos;
}
);

// -- Fragment shader ----------------------------------------------------------

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

static WorldState* state = nullptr;
static TraceOptions* traceOptions = nullptr;

// Animation/transformation stuff //////////////////////////////////////////////

clock_t old_time;
float rotation = 0.0f;

// OpenGL drawing functions ////////////////////////////////////////////////////

static void initShader();
static void cleanupShader();
static void uploadGeometry();
static void cleanup();
static void finish();

static void initPreviewWindow(int argc, char** argv, const std::string& title);
static void display();
static void handleWindowResize(GLFWwindow* window, int width, int height);
static void handleError(int error, const char* description);
static void handleKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods);

////////////////////////////////////////////////////////////////////////////////
// Options:
////////////////////////////////////////////////////////////////////////////////

enum OptionIndex { 
    UNKNOWN
    ,HELP
    ,RUN_TESTS_AND_QUIT
    ,PRINT_CONFIG_AND_QUIT
    ,PRINT_CAMERA
    ,DEBUG_PIXEL
    ,DISABLE_PREVIEW
    ,SAMPLES_PER_LIGHT
    ,SAMPLES_PER_PIXEL
};

const option::Descriptor usage[] =
{
    {
         UNKNOWN
        ,0
        ,"" 
        ,""
        ,option::Arg::None
        ,"USAGE: <program> [options]\n\n Options:" 
    },
    {
         HELP
        ,0
        ,""
        ,"help"
        ,option::Arg::None
        ,"  --help  \t\tPrint usage and exit."
    },
    {
         RUN_TESTS_AND_QUIT
        ,0
        ,"T"
        ,"tests"
        ,option::Arg::None
        ,"  --tests \t\tRun tests and exit."
    },
    {
         PRINT_CONFIG_AND_QUIT
        ,0
        ,"C"
        ,"config"
        ,option::Arg::None
        ,"  --config \t\tPrint the configuration to stdout and quit."
    },
    {
         PRINT_CAMERA
        ,0
        ,""
        ,"camera"
        ,option::Arg::None
        ,"  --camera \t\tPrint the camera to stdout."
    },
    {
         DEBUG_PIXEL
        ,0
        ,"D"
        ,"debug"
        ,option::Arg::Optional
        ,"  --debugX --debugY or -D240 -D 130 \t\tDebugs the output for pixel at coordinate <x,y>."
    },
    {
         DISABLE_PREVIEW
        ,0
        ,"P"
        ,"no-preview"
        ,option::Arg::None
        ,"  -P/--no-preview \t\tDisable preview mode and begin rendering immediately."
    },
    {
         SAMPLES_PER_LIGHT
        ,0
        ,"S"
        ,"light-samples"
        ,option::Arg::Optional
        ,"  -S/--light-samples \t\tSpecifies the number of shadow rays to be sampled."
    },
    {
         SAMPLES_PER_PIXEL
        ,0
        ,"A"
        ,"aa"
        ,option::Arg::Optional
        ,"  -A/--aa \t\tSpecifies the number of primary rays used to sample each pixel."
    },
    {
         UNKNOWN
        ,0
        ,""
        ,""
        ,option::Arg::None
        ,"\nExamples:\n"
         "  example --unknown -- --this_is_no_option\n"
         "  example -unk --plus -ppp file1 file2\n" 
    },
    {0, 0, 0, 0, 0, 0}
};

/**
 *
 */
static void runRaytracer(TraceOptions options, bool disablePreview = false)
{
    rayTrace(output
            ,*state
            ,rayTraceCamera
            ,state->getConfiguration().getSceneGraph()
            ,state->getConfiguration().RESO[0]
            ,state->getConfiguration().RESO[1]
            ,options);

    // Only generate output if a debug pixel has not been set:
    if (!options.enablePixelDebug) {
        string outputFile = Utils::cwd("output.bmp");
        output.WriteToFile(outputFile.c_str());
        cout << "Output written to " << outputFile << endl;
    
    } else {

        if (disablePreview) {
            finish();
        }
    }
}

/**
 *
 */
static void runTestsAndQuit()
{
    RunTests();
    exit(EXIT_SUCCESS);
}

/**
 * Dump the configuration settings to stdout and quit
 */
static void printConfigAndQuit(const Configuration& config)
{
    clog << config << endl;
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

    option::Stats  stats(usage, argc, argv);
    option::Option options[64], buffer[4096];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (parse.error()) {
        exit(EXIT_FAILURE);
    }

    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        exit(EXIT_SUCCESS);
    }

    // Seed PRNG
    srand((unsigned int)time(nullptr));

    // Run tests
    if (options[RUN_TESTS_AND_QUIT]) {
        runTestsAndQuit();
    }

    // Parse configuration
    Configuration config(argv[argc-1]);
    try {
        config.read();
    } catch (std::runtime_error& e) {
        cerr << "[!] Configuration reader error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    // Dump configuration to stdout
    if (options[PRINT_CONFIG_AND_QUIT]) {
        printConfigAndQuit(config);
    }

    // Create the world state object from the current configuration:
    state = new WorldState(config);

    // Configure options used during ray tracing
    traceOptions = new TraceOptions();

    // Samples per pixel:
    if (options[SAMPLES_PER_PIXEL].count() > 0) {
        const char* str = options[SAMPLES_PER_PIXEL].first()->arg;
        if (str != nullptr) {
            traceOptions->samplesPerPixel = Utils::parseNumber(string(str), TraceOptions::SAMPLES_PER_PIXEL_DEFAULT);
        } else {
            traceOptions->samplesPerPixel = TraceOptions::SAMPLES_PER_PIXEL_DEFAULT;
        }
    }

    // Samples per light:
    if (options[SAMPLES_PER_LIGHT].count() > 0) {
        const char* str = options[SAMPLES_PER_LIGHT].first()->arg;
        if (str != nullptr) {
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
    initRaytrace(config, *state, rayTraceCamera);
    output.SetBitDepth(24);
    output.SetSize(state->getWindowWidth(), state->getWindowHeight());

    if (options[PRINT_CAMERA]) {
        clog << rayTraceCamera << endl;
    }

    // If no preview, rendering starts immediately:
    if (options[DISABLE_PREVIEW]) {
        runRaytracer(*traceOptions, true);
    } else {
        initPreviewWindow(argc, argv, "OpenGL Preview");
    }

    exit(EXIT_SUCCESS);
}

/**
 * Initializes the OpenGL preview window
 */
void initPreviewWindow(int argc, char** argv, const string& title)
{
    GLFWwindow* window = nullptr;

    #ifdef DEBUG
    clog << "- initPreviewWindow()" << endl;
    #endif

    if (!glfwInit()) {
        cerr << "glfwInit() failed" << endl;
        exit(EXIT_FAILURE);
    }

    // Force at least OpenGL 3.2 on Mac by using the "Core" profile:    
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(state->getWindowWidth(), state->getWindowHeight(), title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        cerr << "glfwCreateWindow() failed" << endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cerr << "glewInit() failed" << endl;
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

    handleWindowResize(window, state->getWindowWidth(), state->getWindowHeight());

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
    clog << "- cleanup()" << endl;
    #endif

    glDeleteProgram(shaderProgram);

    if (state != nullptr) {
        delete state;
        state = nullptr;
    }
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
    #ifdef DEBUG
    clog << "- initShader()" << endl;
    #endif

    // Tell the GPU to create new shaders and a shader program
    GLuint shadVert = glCreateShader(GL_VERTEX_SHADER);
    GLuint shadFrag = glCreateShader(GL_FRAGMENT_SHADER);
    shaderProgram   = glCreateProgram();

    // Load and compile each shader program and check to make sure 
    // the shaders complied correctly

    // - Vertex shader
    glShaderSource(shadVert, 1, &vertexShader, nullptr);
    glCompileShader(shadVert);
    GLUtils::printShaderInfoLog(clog, shadVert);

    // - Diffuse fragment shader
    glShaderSource(shadFrag, 1, &fragmentShader, nullptr);
    glCompileShader(shadFrag);
    GLUtils::printShaderInfoLog(clog, shadFrag);

    // Link the shader programs together from compiled bits
    glAttachShader(shaderProgram, shadVert);
    glAttachShader(shaderProgram, shadFrag);
    glBindFragDataLocation(shaderProgram, 0, "out_Color");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    GLUtils::printLinkInfoLog(clog, shaderProgram);

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

    GLUtils::printErrorLog(cerr);
}

/**
 * Uploads the geometry contained in the given scene graph node
 */
static void* uploadNode(GraphNode* node, void* ignore, int depth)
{
    #ifdef DEBUG
    clog << "-- uploadNode()" << endl;
    #endif

    GLGeometry* instance = node->getInstance();

    if (instance != nullptr) {
        instance->upload(shaderProgram, locationPos, locationNor, locationCol);
    }

    return nullptr;
}

/**
 * Walks the scene graph, uploading the geometry contained in the graph's nodes
 */
void uploadGeometry()
{
    #ifdef DEBUG
    clog << "- uploadGeometry()" << endl;
    #endif

    GraphNode* root = state->getConfiguration().getSceneGraph().getRoot();

    if (root != nullptr) {
        walk(root, uploadNode, (void*)nullptr);
    }
}


/** 
 * Transforms the given affine matrix current according to the geometries
 * position in the scene graph, then draws tha actual geometry using the
 * updated affine matrix
 */
static glm::mat4 drawGLGeometry(GraphNode* node, glm::mat4 current, int depth)
{
    glm::mat4 next = applyTransform(node, current);

    if (state->doRotateScene() && node->isRoot()) {
        next *= glm::rotate(glm::mat4(), rotation, glm::vec3(0.0, 1.0f, 0.0f));
    }

    GLGeometry* instance = node->getInstance();

    if (instance != nullptr) {
        instance->draw(state, shaderProgram, unifModel, unifModelInvTr, next);
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
    glm::vec3 eyePosition = state->getEyePosition();
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

    GraphNode* root = state->getConfiguration().getSceneGraph().getRoot();

    if (root != nullptr) {
        walk(root, drawGLGeometry, glm::mat4());
    }

    // -------------------------------------------------------------------------

    GLUtils::printErrorLog(cerr);
}

/**
 * Handler for window resizing
 */
void handleWindowResize(GLFWwindow* window, int width, int height)
{
    #ifdef DEBUG
    clog << "- handleWindowResize() = <" << width << "," << height << ">" << endl;
    #endif

    glViewport(0, 0, width, height);

    glm::mat4 projection = glm::perspective(glm::radians(state->getFOVAngle())
                                           ,state->getAspectRatio()
                                           ,state->getZNear()
                                           ,state->getZFar());

    glm::mat4 camera = glm::lookAt(state->getEyePosition()
                                  ,state->getLookAtPosition()
                                  ,state->getUpDirection());

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
    cerr << "Error: " << description << endl;
}

/**
 * Handler for key presses
 */
void handleKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    #ifdef DEBUG
    clog << "- handleKeyPress() = '" << key << "'" << endl;
    #endif
    switch (key) {
        case GLFW_KEY_P:
            {
                if (action == GLFW_PRESS) {
                    runRaytracer(*traceOptions, true);
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

