/*
 * example3.c
 * By Steven Smith
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sst.h"

static const char *shaders[] = {"shaders/test2.vert", "shaders/test2.frag"};
static const int shader_count = 2;

static GLfloat positions[] = {  1.0f,  1.0f,  1.0f,
                                1.0f,  1.0f, -1.0f,
                               -1.0f,  1.0f, -1.0f,
                               -1.0f,  1.0f,  1.0f,
                                1.0f, -1.0f,  1.0f,
                                1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f,  1.0f };

#define TRIANGLE_COUNT 36

static int triangles[] = { 3, 2, 1,
                           1, 2, 5,
                           2, 6, 5,
                           4, 5, 6,
                           7, 4, 6,
                           2, 3, 7,
                           7, 6, 2,
                           1, 5, 4,
                           4, 7, 3,
                           4, 3, 0,
                           0, 1, 4,
                           3, 1, 0 };

/* Data managing code */

static GLfloat * generateVertices() {
    GLfloat *result;
    unsigned int i;
    result = (GLfloat*)malloc(sizeof(GLfloat) * TRIANGLE_COUNT * 3);
    for( i = 0; i < TRIANGLE_COUNT; i++ ) {
        result[i*3] = positions[triangles[i]*3];
        result[i*3 + 1] = positions[triangles[i]*3 + 1];
        result[i*3 + 2] = positions[triangles[i]*3 + 2];
    }
    return result;
}

static GLfloat * generateNormals( GLfloat *verts ) {
    GLfloat *result;
    unsigned int i;
    result = (GLfloat*)malloc(sizeof(GLfloat) * TRIANGLE_COUNT * 3);
    memcpy(result, verts, sizeof(GLfloat) * TRIANGLE_COUNT * 3);
    for( i = 0; i < TRIANGLE_COUNT; i++ ) {
        sstNormalize3_(&result[i*3]);
    }
    return result;
}

/* Main loop */

static void rotateMovement( GLfloat x, GLfloat y, GLfloat rot, GLfloat *dx,
GLfloat *dy ) {
    GLfloat sinRot, cosRot;
    sinRot = sin(rot);
    cosRot = cos(rot);
    *dx += (x * cosRot) - (y * sinRot);
    *dy += (y * cosRot) + (x * sinRot);
}

int mainLoop( GLFWwindow window, sstProgram *program, sstDrawableSet *set ) {
    GLfloat model[16], temp[16];
    GLfloat x, y, rotx, roty, dx, dy;
    int mX, mY, mXl, mYl;
    x = y = 0.0f;
    dx = 0.0f;
    dy = -100.0f;
    mX = mY = mXl = mYl = -1;
    rotx = roty = 0.0f;
    while( 1 ) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /* Update the look-at values */
        rotx += ((GLfloat) (mX - mXl)) / 25.0f;
        roty += ((GLfloat) (mY - mYl)) / 25.0f;
        /* Matrix twiddling */
        sstRotateMatrixX_(roty, model);
        sstRotateMatrixY_(rotx, temp);
        sstMatMult4_(model, temp, model);
        rotateMovement(x, y, rotx, &dx, &dy);
        sstTranslateMatrixInto(dx, 0.0f, dy, model);
        sstScaleMatrixInto(20.0f, 20.0f, 20.0f, model);
        /* Set our model uniform and draw */
        sstSetUniformData(program, "modelMatrix", model);
        sstDrawSet(set);
        /* Set up matrices for second cube */
        sstTranslateMatrixInto(0.0f, 0.0f, -5.0f, model);
        /* Draw second cube */
        sstSetUniformData(program, "modelMatrix", model);
        sstDrawSet(set);
        /* Check for errors and clean up */
        if( sstDisplayErrors() ) {
            return 1;
        }
        glFlush();
        glfwSwapBuffers(window);
        glfwPollEvents();
        /* Quitting */
        if( glfwGetKey(window, GLFW_KEY_ESC)
         || glfwGetWindowParam(window, GLFW_CLOSE_REQUESTED) ) {
            return 0;
        }
        /* Movement */
        if( glfwGetKey(window, 'W') == GLFW_PRESS ) {
            y = 1.0f;
        }
        else if( glfwGetKey(window, 'S') == GLFW_PRESS ) {
            y = -1.0f;
        }
        else {
            y = 0.0f;
        }
        if( glfwGetKey(window, 'A') == GLFW_PRESS ) {
            x = 1.0f;
        }
        else if( glfwGetKey(window, 'D') == GLFW_PRESS ) {
            x = -1.0f;
        }
        else {
            x = 0.0f;
        }
        /* Mouse looking */
        if( glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ) {
            if( mXl == -1 ) {
                glfwGetCursorPos(window, &mXl, &mYl);
            }
            else {
                mXl = mX;
                mYl = mY;
            }
            glfwGetCursorPos(window, &mX, &mY);
        }
        else if( glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE ) {
            mX = mY = mXl = mYl = -1;
        }
    }
    return 1;
}

/* Setup */

GLFWwindow initialize() {
    GLFWwindow window;
    /* Hard-coded values for now */
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    window = glfwCreateWindow(600, 600, GLFW_WINDOWED, "GLFW Window", NULL);
    if( window == NULL ) {
        printf("Failed to open window!\n");
        printf("Error: %s\n", glfwErrorString(glfwGetError()));
        return NULL;
    }
    glfwMakeContextCurrent(window);
    return window;
}

int dataSetup( GLFWwindow window ) {
    sstProgram *program;
    sstDrawableSet *set;
    GLfloat *verts, *norms, *proj;
    int result;
    /* Create shader program */
    program = sstNewProgram(shaders, shader_count);
    if( !program ) {
        printf("Failed to start: couldn't create program!\n");
        return 1;
    }
    /* Set up data */
    verts = generateVertices();
    norms = generateNormals(verts);
    proj = sstPerspectiveMatrix(60.0f, 1.0f, 5.0f, 505.0f);
    sstActivateProgram(program);
    set = sstGenerateDrawableSet(program, GL_TRIANGLES, TRIANGLE_COUNT,
                                 "in_Position", verts,
                                 "in_Normal", norms);
    sstSetUniformData(program, "projectionMatrix", proj);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 600, 600);
    result = mainLoop(window, program, set);
    free(verts);
    free(norms);
    free(proj);
    return result;
}

int glfwContext() {
    GLFWwindow window;
    int result;
    window = initialize();
    if( window ) {
        result = dataSetup(window);
    }
    else {
        result = 1;
    }
    glfwTerminate();
    return result;
}

int initContext() {
    if( !glfwInit() ) {
        printf("Failed to init GLFW!\n");
        return 1;
    }
    if( glfwContext() ) {
        return 1;
    }
    else {
        return 0;
    }
}

int main( void ) {
    if( initContext() ) {
        exit(EXIT_FAILURE);
    }
    else {
        exit(EXIT_SUCCESS);
    }
    return 0;
}
