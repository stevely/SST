/*
 * sst.h
 * By Steven Smith
 */

#ifndef SST_H_
#define SST_H_

/*
 * Stuff from sst.c
 */
#include <stdarg.h>

/* Needed to get OpenGL 3+ bindings */
#define GLFW_INCLUDE_GLCOREARB
#include <GL/glfw3.h>

typedef struct {
    char *name;
    GLint location;
    GLenum type;
    GLuint size; /* Size of component, ie. sizeof(GLFLOAT) */
    GLuint components; /* Number of values per entry, ie. 3 for vec3 */
} in_var;

typedef struct {
    char *name;
    GLint location;
    GLenum type;
    GLuint first; /* Number of columns per entry, ie. 3 for vec3 and mat3 */
    GLuint second; /* For matrices, number of rows. 0 otherwise */
    GLboolean transpose; /* Only for matrices */
    GLuint count;
} uniform;

typedef struct {
    in_var *inputs;
    int in_count;
    uniform *uniforms;
    int un_count;
    GLuint *shaders;
    int shader_count;
    GLuint program; /* Program ID */
} sstProgram;

typedef struct {
    GLuint buffer; /* Buffer location */
    GLint location; /* Attribute location */
    GLuint components;
    GLenum type;
    GLboolean transpose;
} sstDrawable;

typedef struct {
    GLuint vao; /* Vertex array object for this set */
    int count; /* Number of inputs stored in the buffers */
    int size; /* Number of drawables */
    sstDrawable *drawables;
} sstDrawableSet;

/*
 * Displays any OpenGL errors to stdout. Since some of the error types have been
 * removed in later versions of OpenGL, we check if they exist in the
 * preprocessor.
 */
GLenum sstDisplayErrors();

/*
 * Creates a program object, including compiling and linking the given shader
 * programs, as well as parsing the shader programs and pulling out the relevent
 * data.
 */
sstProgram * sstNewProgram( const char **files, int count );

/*
 * Activates the given program, grabbing all of its input and uniform
 * variables and making the program the active OpenGL program.
 */
void sstActivateProgram( sstProgram *program );

/*
 * Generates a drawable set. This function takes in an sstProgram, the number of
 * component values for the set, and a number of pair values consisting of the
 * name of an input variable in the program followed by its data.
 * Note that the count is the number of items in the dataset relative to its
 * GLSL type. Eg. given an array of six floats representing the dataset for a
 * series of 'vec3' values, count would be 2 because there are 2 'vec3's being
 * passed in.
 */
sstDrawableSet * sstGenerateDrawableSet( sstProgram *program, int count, ... );

/*
 * Draws the given sstDrawableSet. Assumes the correct program is currently
 * active.
 */
void sstDrawSet( sstDrawableSet *set );

/*
 * Sets the given uniform variable to the given value.
 * DEV NOTE: Some of these functions are only defined in OpenGL versions later
 * than 3.2. Since I can't #ifdef to check their existance ahead of time, they
 * are commented out until I can think of a better solution.
 */
void sstSetUniformData( sstProgram *program, char *name, GLvoid *data );

/*
 * Frees the given sstDrawableSet object, deleting with it all related OpenGL
 * objects.
 */
void sstFreeDrawableSet( sstDrawableSet *set );

/*
 * Frees the given sstProgram object, deleting with it all related OpenGL
 * objects.
 */
void sstFreeProgram( sstProgram *program );

/*
 * Helper functions for dealing with matrices and vectors. The memory policy is
 * all allocated belongs to the caller, functions ending with '_' take in a
 * vector or matrix to populate with values. These functions are safe for
 * over-writing values passed in as arguments.
 * Defined in sst_matrix.c.
 */

GLfloat * sstIdentityMatrix4x4 ();
void      sstIdentityMatrix4x4_( GLfloat *mat );

GLfloat * sstDupMatrix4x4 ( GLfloat *src );
void      sstDupMatrix4x4_( GLfloat *src, GLfloat *dst );

GLfloat * sstDupMatrix3x3 ( GLfloat *src );
void      sstDupMatrix3x3_( GLfloat *src, GLfloat *dst );

GLfloat * sstIdentityMatrix3x3 ();
void      sstIdentityMatrix3x3_( GLfloat *mat );

GLfloat * sstPerspectiveMatrix ( GLfloat fovy, GLfloat aspect, GLfloat znear,
                                 GLfloat zfar );
void      sstPerspectiveMatrix_( GLfloat fovy, GLfloat aspect, GLfloat znear,
                                 GLfloat zfar, GLfloat *mat );

GLfloat * sstTranslateMatrix    ( GLfloat x, GLfloat y, GLfloat z );
void      sstTranslateMatrix_   ( GLfloat x, GLfloat y, GLfloat z, GLfloat *mat );
void      sstTranslateMatrixInto( GLfloat x, GLfloat y, GLfloat z, GLfloat *mat );

GLfloat * sstRotateMatrix  ( GLfloat theta, GLfloat x, GLfloat y, GLfloat z );
void      sstRotateMatrix_ ( GLfloat theta, GLfloat x, GLfloat y, GLfloat z,
                             GLfloat *mat );
void      sstRotateMatrixX_( GLfloat theta, GLfloat *mat );
void      sstRotateMatrixY_( GLfloat theta, GLfloat *mat );
void      sstRotateMatrixZ_( GLfloat theta, GLfloat *mat );

GLfloat * sstScaleMatrix    ( GLfloat x, GLfloat y, GLfloat z );
void      sstScaleMatrix_   ( GLfloat x, GLfloat y, GLfloat z, GLfloat *mat );
void      sstScaleMatrixInto( GLfloat x, GLfloat y, GLfloat z, GLfloat *mat );

GLfloat * sstCrossProduct3 ( GLfloat *v1, GLfloat *v2 );
void      sstCrossProduct3_( GLfloat *v1, GLfloat *v2, GLfloat *target );

GLfloat sstDotProduct3( GLfloat *v1, GLfloat *v2 );

GLfloat * sstNormalize3 ( GLfloat *v );
void      sstNormalize3_( GLfloat *v );

GLfloat * sstMatMult3 ( GLfloat *m1, GLfloat *m2 );
void      sstMatMult3_( GLfloat *m1, GLfloat *m2, GLfloat *target );

GLfloat * sstMatMult4 ( GLfloat *m1, GLfloat *m2 );
void      sstMatMult4_( GLfloat *m1, GLfloat *m2, GLfloat *target );

#endif
