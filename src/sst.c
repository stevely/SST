/*
 * sst.c
 * By Steven Smith
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sst.h"

#define CHUNK_SIZE 512

/*
 * Displays any OpenGL errors to stdout. Since some of the error types have been
 * removed in later versions of OpenGL, we check if they exist in the
 * preprocessor.
 */
GLenum sstDisplayErrors() {
    GLenum err;
    err = glGetError();
    switch( err ) {
    case GL_INVALID_ENUM:
        printf("Invalid enum value!\n");
        break;
    case GL_INVALID_VALUE:
        printf("Numeric argument out of range!\n");
        break;
    case GL_INVALID_OPERATION:
        printf("An invalid operation has occured!\n");
        break;
#ifdef GL_STACK_OVERFLOW
    case GL_STACK_OVERFLOW:
        printf("Stack overflow!\n");
        break;
#endif
#ifdef GL_STACK_UNDERFLOW
    case GL_STACK_UNDERFLOW:
        printf("Stack underflow!\n");
        break;
#endif
    case GL_OUT_OF_MEMORY:
        printf("Out of memory!\n");
        break;
#ifdef GL_TABLE_TOO_LARGE
    case GL_TABLE_TOO_LARGE:
        printf("Table exceeds maximum table size!\n");
        break;
#endif
    default:
        break;
    }
    return err;
}

/*
 * Returns the shader type given a file path. Shader type is determined by the
 * file prefix.
 */
static GLenum sstGetShaderTypeFromFilepath( const char *file ) {
    char *suffix;
    suffix = strrchr(file, '.'); /* Last . -> file suffix */
    /* Compare against potential suffixes to find type */
    if(      strcmp(suffix, ".vert") == 0 ) {
        return GL_VERTEX_SHADER;
    }
/* Geometry shaders are only available on later versions of OpenGL */
#ifdef GL_GEOMETRY_SHADER
    else if( strcmp(suffix, ".geom") == 0 ) {
        return GL_GEOMETRY_SHADER;
    }
#endif
    else if( strcmp(suffix, ".frag") == 0 ) {
        return GL_FRAGMENT_SHADER;
    }
    else {
        printf("Unknown shader file suffix: %s\n", suffix);
        /* Defaulting to vertex on lookup failure for now */
        return GL_VERTEX_SHADER;
    }
}

/*
 * Returns the size of the component corresponding to the enum value given.
 */
static GLuint sstSizeFromEnum( GLenum type ) {
    switch( type ) {
    case GL_BYTE:                        return sizeof(GLbyte);
    case GL_UNSIGNED_BYTE:               return sizeof(GLubyte);
    case GL_SHORT:                       return sizeof(GLshort);
    case GL_UNSIGNED_SHORT:              return sizeof(GLushort);
    case GL_INT:                         return sizeof(GLint);
    case GL_UNSIGNED_INT:                return sizeof(GLuint);
    case GL_HALF_FLOAT:                  return sizeof(GLshort); /* Guess */
    case GL_FLOAT:                       return sizeof(GLfloat);
    case GL_DOUBLE:                      return sizeof(GLdouble);
#ifdef GL_INT_2_10_10_10_REV
    case GL_INT_2_10_10_10_REV:          return sizeof(GLint); /* Guess */
#endif
    case GL_UNSIGNED_INT_2_10_10_10_REV: return sizeof(GLuint); /* Guess */
    default:
        printf("WARN: Unrecognized GL type value: %d\n", type);
        return sizeof(GLint);
    }
}

/*
 * For efficient rendering we use arrays of in and uniform structs. However, we
 * use linked lists to build up our data set so we can convert it into arrays.
 */

typedef struct in_var_list {
    in_var value;
    struct in_var_list *next;
} in_var_list;

typedef struct uniform_list {
    uniform value;
    struct uniform_list *next;
} uniform_list;

/*
static void sstPrintInputs( in_var_list *list ) {
    printf("INPUT VARIABLE LIST:\n");
    while( list ) {
        printf("NAME: %s | SIZE: %d | COMPONENTS: %d\n", list->value.name,
            list->value.size, list->value.components);
        list = list->next;
    }
    printf("\n");
}

static void sstPrintUniforms( uniform_list *list ) {
    printf("UNIFORM VARIABLE LIST:\n");
    while( list ) {
        printf("NAME: %s | FIRST: %d | SECOND: %d\n", list->value.name,
            list->value.first, list->value.second);
        list = list->next;
    }
    printf("\n");
}
*/

/*
 * Helper parser functions
 */

/*
 * Returns true iff the given character is a valid character for an identifier.
 */
static int sstIsIdentChar( char c ) {
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || (c == '_');
}

/*
 * Given a pointer to a string, return a copy of the string until the first non-
 * identifier character.
 */
static char * sstCopyName( char *string ) {
    char *result, *s;
    int count;
    s = string;
    /* Step 1: Find the end of the identifier */
    for( count = 0; sstIsIdentChar(*s); count++, s++ );
    /* Step 2: Create copy string */
    result = (char*)malloc(sizeof(char) * (count + 1));
    strncpy(result, string, count);
    result[count] = '\0';
    /* Step 3: Return result */
    return result;
}

/*
 * GLSL data types:
 * float
 * double
 * bool
 * int
 * uint
 * vec{2,3,4} floats
 * dvec{2,3,4} doubles
 * bvec{2,3,4} booleans
 * ivec{2,3,4} integers
 * uvec{2,3,4} unsigned integers
 * mat2, dmat2
 * mat3, dmat3
 * mat4, dmat4
 * mat{2,3,4}x{2,3,4}
 * dmat{2,3,4}x{2,3,4}
 */

/*
 * Helper function for parsing matrix types.
 */
static void sstParseMatrix( char *s, GLuint *first, GLuint *second ) {
    s += 3;
    /* This increments s, but evaluates to s before the increment */
    switch( *(s++) ) {
    case '2':
        *first = 2;
        break;
    case '3':
        *first = 3;
        break;
    case '4':
        *first = 4;
        break;
    default:
        printf("WARN: Bad matrix component: %c\n", *(s-1));
        return;
    }
    /* Potentially non-square matrix */
    if( *(s++) == 'x' ) {
        switch( *s ) {
        case '2':
            *second = 2;
            break;
        case '3':
            *second = 3;
            break;
        case '4':
            *second = 4;
            break;
        default:
            printf("WARN: Bad matrix secondary component: %c\n", *s);
            return;
        }
    }
    /* Square matrix */
    else {
        *second = *first;
    }
}

/*
 * Helper function for parsing vector types.
 */
static GLuint sstParseVector( char *s ) {
    if( strncmp(s, "vec", 3) == 0 ) {
        s += 3;
        switch( *s ) {
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        default:
            printf("WARN: Bad vector component: %c\n", *s);
            return 0;
        }
    }
    return 0;
}

/*
 * Takes a string and identifies the GLSL type, returning the type of each
 * component and the number of components through the two values passed by
 * reference.
 * NOTE: This does not check if the type is an array. It also will not reject
 * every bad data type (notably it will erroneously succeed if the type has a
 * prefix of a valid type, ie. "mat33" will be treated as having type "mat3")
 */
static void sstParseType( char *s, GLenum *type, GLuint *first, GLuint *second ) {
    *type = 0;
    *first = 0;
    *second = 0;
    switch( *s ) {
    case 'm':
        /* Matrix */
        if( strncmp(s, "mat", 3) == 0 ) {
            *type = GL_FLOAT; /* mat = floats */
            sstParseMatrix(s, first, second);
        }
        else {
            printf("WARN: Unknown data type.\n");
        }
        return;
    case 'd':
        /* Matrix with doubles */
        if( strncmp(s, "dmat", 4) == 0 ) {
            *type = GL_DOUBLE; /* dmat = doubles */
            s++; /* Need to start the string at the 'm' in 'mat' */
            sstParseMatrix(s, first, second);
        }
        /* Double */
        else if( strncmp(s, "double", 6) == 0 ) {
            *type = GL_DOUBLE;
            *first = 1;
        }
        /* Vector with doubles */
        else if( (*first = sstParseVector(s+1)) != 0 ) {
            *type = GL_DOUBLE;
        }
        else {
            printf("WARN: Unknown data type.\n");
        }
        return;
    case 'v':
        /* Vector */
        if( (*first = sstParseVector(s)) != 0 ) {
            *type = GL_FLOAT;
        }
        else {
            printf("WARN: Unknown data type.\n");
        }
        return;
    case 'b':
        /* Vector with booleans */
        if( (*first = sstParseVector(s+1)) != 0 ) {
            *type = GL_BYTE; /* Booleans are stored as bytes */
        }
        /* Boolean */
        else if( strncmp(s, "bool", 4) == 0 ) {
            *type = GL_BYTE; /* Booleans are stored as bytes */
            *first = 1;
        }
        else {
            printf("WARN: Unknown data type.\n");
        }
        return;
    case 'i':
        /* Vector with integers */
        if( (*first = sstParseVector(s+1)) != 0 ) {
            *type = GL_INT;
        }
        /* Integer */
        else if( strncmp(s, "int", 3) == 0 ) {
            *type = GL_INT;
            *first = 1;
        }
        else {
            printf("WARN: Unknown data type.\n");
        }
        return;
    case 'u':
        /* Vector with unsigned integers */
        if( (*first = sstParseVector(s+1)) != 0 ) {
            *type = GL_UNSIGNED_INT;
        }
        /* Unsigned integer */
        else if( strncmp(s, "uint", 4) == 0 ) {
            *type = GL_UNSIGNED_INT;
            *first = 1;
        }
        else {
            printf("WARN: Unknown data type.\n");
        }
        return;
    default:
        printf("WARN: Unknown data type.\n");
        return;
    }
}

/*
 * Assuming that the string s is pointing 1 character past the end of the
 * identifier, parses the string and returns the number of array components.
 * If there are no array components, it returns 1.
 */
static GLuint sstParseArray( char *s ) {
    GLuint count;
    if( *s == '[' ) {
        s++;
        for( count = 0; *s >= '0' && *s <= '9'; s++ ) {
            count *= 10;
            count += *s - '0';
        }
        if( *s != ']' ) {
            printf("WARN: Unexpected character while parsing array: %c\n", *s);
            count = 0;
        }
    }
    else {
        count = 1;
    }
    return count;
}

static in_var_list *ins = NULL;
static in_var_list *ins_end = NULL;
static uniform_list *uns = NULL;
static uniform_list *uns_end = NULL;

static void sstAppendInputList( char *name, GLenum type, GLuint components ) {
    in_var_list *result;
    result = (in_var_list*)malloc(sizeof(in_var_list));
    result->value.name = name;
    result->value.location = 0;
    result->value.type = type;
    result->value.size = sstSizeFromEnum(type);
    result->value.components = components;
    result->next = NULL;
    if( ins == NULL ) {
        ins = ins_end = result;
    }
    else {
        ins_end->next = result;
        ins_end = result;
    }
}

static void sstAppendUniformList( char *name, GLenum type, GLuint first,
GLuint second, GLuint count ) {
    uniform_list *result, *check;
    /* Check if the uniform already exists (is defined in another shader) */
    for( check = uns; check; check = check->next ) {
        if( strcmp(name, check->value.name) == 0 ) {
            /* The linker will check that the types match, so we can safely
             * ignore the duplicates. */
            return;
        }
    }
    result = (uniform_list*)malloc(sizeof(uniform_list));
    result->value.name = name;
    result->value.location = 0;
    result->value.type = type;
    result->value.first = first;
    result->value.second = second;
    result->value.transpose = GL_TRUE; /* Always transpose for now */
    result->value.count = count;
    result->next = NULL;
    if( uns == NULL ) {
        uns = uns_end = result;
    }
    else {
        uns_end->next = result;
        uns_end = result;
    }
}

/*
 * Helper function for sstParseLine()
 */
static void sstParseLine1( char *s, char **name, GLenum *type, GLuint *first,
GLuint *second, GLuint *count ) {
    /* Skip whitespace */
    while( !sstIsIdentChar(*s) ) {
        s++;
    }
    /* Grab type */
    sstParseType(s, type, first, second);
    /* Skip type identifier */
    while( sstIsIdentChar(*s) ) {
        s++;
    }
    /* Skip whitespace */
    while( !sstIsIdentChar(*s) ) {
        s++;
    }
    /* Grab identifier name */
    *name = sstCopyName(s);
    /* Skip name */
    while( sstIsIdentChar(*s) ) {
        s++;
    }
    /* Grab count for arrays */
    *count = sstParseArray(s);
}

/*
 * NOTE: There are two versions of the sstParseLine function. One checks for input
 * variables, one doesn't. This is because only vertex shaders have input
 * variables that are fed in from the host program.
 */

/*
 * Parses a line for either an input variable or a uniform variable. If one is
 * found, it is added to their respective list.
 */
static void sstParseLineWithInputs( char *s ) {
    char *name;
    GLenum type;
    GLuint first, second;
    GLuint count;
    /* Input variables */
    if( strncmp(s, "in ", 3) == 0 ) {
        s += 3;
        sstParseLine1(s, &name, &type, &first, &second, &count);
        /* Add to the input list */
        if( second == 0 ) { /* second == 0 -> not a matrix */
            sstAppendInputList(name, type, first * count);
        }
        else {
            sstAppendInputList(name, type, first * second * count);
        }
    }
    /* Uniform variables */
    else if( strncmp(s, "uniform ", 8) == 0 ) {
        s += 8;
        sstParseLine1(s, &name, &type, &first, &second, &count);
        /* Add to the uniform list */
        sstAppendUniformList(name, type, first, second, count);
    }
}

/*
 * Parses a line for a uniform variable. If one is found, it is added to the
 * uniform variable list.
 */
static void sstParseLine( char *s ) {
    char *name;
    GLenum type;
    GLuint first, second;
    GLuint count;
    if( strncmp(s, "uniform ", 8) == 0 ) {
        s += 8;
        sstParseLine1(s, &name, &type, &first, &second, &count);
        /* Add to the uniform list */
        sstAppendUniformList(name, type, first, second, count);
    }
}

/*
 * Given an array of strings representing the source of a shader program, parse
 * each line for input and uniform variables, adding them to their respective
 * lists. Takes the shader type to determine if input variables should be parsed
 * or not.
 */
static void sstParseShader( GLenum type, char **strings, int count ) {
    char *s, *c;
    int i;
    char chunk[CHUNK_SIZE]; /* Temp buffer for reading lines */
    c = chunk;
    /* Iterate through our array of strings */
    for( i = 0; i < count; i++ ) {
        s = strings[i];
        while( *s != '\0' ) {
            /* Check for newlines or end-of-chunk */
            while( *s != '\n' && *s != '\0' ) {
                *(c++) = *(s++);
            }
            /* If it's a newline or if it's the end of the last chunk, parse it */
            if( *s == '\n' || i+1 == count ) {
                *c = '\0';
                s++;
                if( type == GL_VERTEX_SHADER ) {
                    sstParseLineWithInputs(chunk);
                }
                else {
                    sstParseLine(chunk);
                }
                c = chunk;
            }
        }
        /* If it's not a newline, we move on to the next chunk */
    }
}

/*
 * Given a shader type and a filepath, read in and attempt to compile a shader
 * program. Will return the ID of the shader program on success, or 0 if there
 * was an error. Will also attempt to parse the program for any input or uniform
 * variables to capture.
 * NOTE: The shader type can be determined from the filepath assuming the usual
 * conventions are kept and the shaders have the appropriate suffix.
 */
static GLuint sstCreateShader( GLenum type, const char *filepath ) {
    GLuint shader;
    FILE *fp;
    char **source;
    int source_size, i;
    size_t bytes_read;
    GLint result;
    GLchar *error;
    GLsizei error_length;
    /* Step 1: Create empty shader */
    shader = glCreateShader(type);
    if( !shader ) {
        printf("Failed to create shader!\n");
        return 0;
    }
    /* Step 2: Read in shader source */
    fp = fopen(filepath, "r");
    if( !fp ) {
        printf("Failed to open shader file %s!\n", filepath);
        return 0;
    }
    /* For the shader source files, we read in chunks of CHUNK bytes, realloc'ing
     * the array of arrays for every extra chunk we need to read in. */
    source = (char**)malloc(sizeof(char*));
    source_size = 1;
    goto skip_realloc;
    while( !feof(fp) ) {
        source_size++;
        source = (char**)realloc(source, sizeof(char*) * source_size);
skip_realloc:
        source[source_size-1] = (char*)malloc(sizeof(char) * CHUNK_SIZE);
        bytes_read = fread(source[source_size-1], sizeof(char), CHUNK_SIZE-1, fp);
        source[source_size-1][bytes_read] = '\0';
    }
    fclose(fp);
    /* lengths = NULL -> NULL-terminated strings */
    glShaderSource(shader, source_size, (const char**)source, NULL);
    /* Step 3: Compile shader */
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if( result == GL_FALSE ) {
        printf("Failed to compile shader %s:\n", filepath);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_length);
        error = (GLchar*)malloc(sizeof(GLchar) * error_length);
        glGetShaderInfoLog(shader, error_length, NULL, error);
        printf("%s\n", error);
        free(error);
        shader = 0;
    }
    /* Step 4: Parse shader for input and uniform variables */
    else {
        /* We do this step after compiling to let the GLSL compiler catch any
         * source errors before we try to parse. */
        sstParseShader(type, source, source_size);
    }
    /* Free our source strings, as we are done with them */
    for( i = 0; i < source_size; i++ ) {
        free(source[i]);
    }
    free(source);
    /* Step 4: Return compiled shader */
    return shader;
}

/*
 * Given an array of filepaths and the size of the array, attempts to compile,
 * parse, and link each shader to create a final shader program. Returns the ID
 * of the program on success, or 0 otherwise.
 */
static void sstCreateProgram( sstProgram *p, const char **files, int count ) {
    GLuint program, shader;
    int i, j;
    GLenum type;
    GLint result;
    GLchar *error;
    GLsizei error_length;
    p->program = 0;
    p->shader_count = count;
    /* Step 1: Create program */
    program = glCreateProgram();
    if( !program ) {
        printf("Failed to create program!\n");
        return;
    }
    /* Step 2: Compile shaders */
    p->shaders = (GLuint*)malloc(sizeof(GLuint) * count);
    for( i = 0; i < count; i++ ) {
        type = sstGetShaderTypeFromFilepath(files[i]);
        shader = sstCreateShader(type, files[i]);
        if( !shader ) {
            for( j = 0; j < i; j++ ) {
                glDeleteShader(p->shaders[j]);
            }
            glDeleteProgram(program);
            return;
        }
        p->shaders[i] = shader;
        glAttachShader(program, shader);
    }
    /* Step 3: Link program */
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if( result == GL_FALSE ) {
        printf("Failed to link program:\n");
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &error_length);
        error = (GLchar*)malloc(sizeof(GLchar) * error_length);
        glGetProgramInfoLog(program, error_length, NULL, error);
        printf("%s\n", error);
        free(error);
        for( i = 0; i < count; i++ ) {
            glDeleteShader(p->shaders[i]);
        }
        glDeleteProgram(program);
        return;
    }
    /* Step 4: Return linked program */
    p->program = program;
}

/*
 * Creates a program object, including compiling and linking the given shader
 * programs, as well as parsing the shader programs and pulling out the relevent
 * data.
 */
sstProgram * sstNewProgram( const char **files, int count ) {
    sstProgram *result;
    int var_count;
    in_var_list *i;
    uniform_list *u;
    in_var *in;
    uniform *un;
    /* Step 1: Create program object */
    result = (sstProgram*)malloc(sizeof(sstProgram));
    /* Step 2: Create program and parse shaders */
    sstCreateProgram(result, files, count);
    if( !result->program ) {
        free(result);
        return NULL;
    }
    /* Step 3: Get input and uniform variable counts */
    var_count = 0;
    i = ins;
    while( i ) {
        var_count++;
        i = i->next;
    }
    result->inputs = (in_var*)malloc(sizeof(in_var) * var_count);
    result->in_count = var_count;
    var_count = 0;
    u = uns;
    while( u ) {
        var_count++;
        u = u->next;
    }
    result->uniforms = (uniform*)malloc(sizeof(uniform) * var_count);
    result->un_count = var_count;
    /* Step 4: Copy over the variables into the new arrays */
    i = ins;
    for( var_count = 0; var_count < result->in_count; var_count++ ) {
        memcpy(&result->inputs[var_count], &i->value, sizeof(in_var));
        ins = i;
        i = i->next;
        free(ins);
    }
    ins = ins_end = NULL;
    u = uns;
    for( var_count = 0; var_count < result->un_count; var_count++ ) {
        memcpy(&result->uniforms[var_count], &u->value, sizeof(uniform));
        uns = u;
        u = u->next;
        free(uns);
    }
    uns = uns_end = NULL;
    /* Step 5: Get locations for inputs */
    for( in = result->inputs; in < result->inputs + result->in_count; in++ ) {
        in->location = glGetAttribLocation(result->program, in->name);
    }
    /* Step 6: Get locations for uniforms */
    for( un = result->uniforms; un < result->uniforms + result->un_count; un++ )
    {
        un->location = glGetUniformLocation(result->program, un->name);
    }
    /* Step 7: Return the program object */
    return result;
}

/*
 * Activates the given program, grabbing all of its input and uniform
 * variables and making the program the active OpenGL program.
 */
void sstActivateProgram( sstProgram *program ) {
    glUseProgram(program->program);
}

/*
 * Generates a drawable set. This function takes in an sstProgram, the number of
 * component values for the set, and a number of pair values consisting of the
 * name of an input variable in the program followed by its data.
 * Note that the count is the number of items in the dataset relative to its
 * GLSL type. Eg. given an array of six floats representing the dataset for a
 * series of 'vec3' values, count would be 2 because there are 2 'vec3's being
 * passed in.
 */
sstDrawableSet * sstGenerateDrawableSet( sstProgram *program, int count, ... ) {
    sstDrawableSet *set;
    sstDrawable *drawable;
    char *name;
    void *data;
    in_var *input;
    va_list ap;
    va_start(ap, count);
    set = (sstDrawableSet*)malloc(sizeof(sstDrawableSet));
    set->size = program->in_count;
    set->count = count;
    /* Step 1: Generate vertex array and bind it */
    glGenVertexArrays(1, &set->vao);
    glBindVertexArray(set->vao);
    /* Step 2: Set up memory for drawables */
    set->drawables = (sstDrawable*)malloc(sizeof(sstDrawable) * set->size);
    for( drawable = set->drawables; drawable < set->drawables + set->size;
         drawable++ ) {
        glGenBuffers(1, &drawable->buffer);
        /* Sub-step 1: Find our data */
        name = va_arg(ap, char*);
        data = va_arg(ap, void*);
        for( input = program->inputs; input < program->inputs + program->in_count;
             input++ ) {
            /* Found match */
            if( strcmp(input->name, name) == 0 ) {
                break;
            }
        }
        /* Lookup failure */
        if( input >= program->inputs + program->in_count ) {
            printf("ERROR: Input variable [%s] does not exist!\n", name);
        }
        /* Sub-step 2: Copy over data */
        drawable->components = input->components;
        drawable->location   = input->location;
        drawable->type       = input->type;
        /* Sub-step 3: Push data down the pipe */
        glBindBuffer(GL_ARRAY_BUFFER, drawable->buffer);
        glBufferData(GL_ARRAY_BUFFER, input->size * input->components * count,
                     data, GL_STATIC_DRAW);
    }
    va_end(ap);
    /* Step 3: Return drawable set */
    return set;
}

/*
 * Draws the given sstDrawableSet. Assumes the correct program is currently
 * active.
 */
void sstDrawSet( sstDrawableSet *set ) {
    sstDrawable *d;
    /* Step 1: Bind our vertex array */
    glBindVertexArray(set->vao);
    /* Step 2: Set up and enable all our input attributes */
    for( d = set->drawables; d < set->drawables + set->size; d++ ) {
        glBindBuffer(GL_ARRAY_BUFFER, d->buffer);
        glVertexAttribPointer(d->location, d->components, d->type, GL_FALSE, 0,
                              0);
        glEnableVertexAttribArray(d->location);
    }
    /* Step 3: Draw arrays */
    glDrawArrays(GL_TRIANGLES, 0, set->count);
    /* Step 4: Disable input attributes */
    for( d = set->drawables; d < set->drawables + set->size; d++ ) {
        glDisableVertexAttribArray(d->location);
    }
}

/*
 * Sets the given uniform variable to the given value.
 * DEV NOTE: Some of these functions are only defined in OpenGL versions later
 * than 3.2. Since I can't #ifdef to check their existance ahead of time, they
 * are commented out until I can think of a better solution.
 */
void sstSetUniformData( sstProgram *program, char *name, GLvoid *data ) {
    uniform *un;
    int i;
    /* Find our data */
    for( i = 0; i < program->un_count; i++ ) {
        un = &program->uniforms[i];
        if( strcmp(un->name, name) == 0 ) {
            break;
        }
    }
    /* Lookup failure */
    if( i >= program->un_count ) {
        printf("WARN: Uniform variable [%s] does not exist!\n", name);
        return;
    }
    switch( un->first ) {
    case 1:
        switch( un->type ) {
//        case GL_FLOAT:
//            glUniformMatrix1fv(un->location, un->count, (const GLfloat*)data);
//            return;
//        case GL_INT:
//            glUniformMatrix1iv(un->location, un->count, (const GLint*)data);
//            return;
//        case GL_UNSIGNED_INT:
//            glUniformMatrix1uiv(un->location, un->count, (const GLuint*)data);
//            return;
        default:
            printf("WARN: Invalid type for uniform value [%s]!\n", name);
            return;
        }
    case 2:
        switch( un->second ) {
        case 0:
        case 1:
            switch( un->type ) {
            case GL_FLOAT:
                glUniformMatrix2fv(un->location, un->count, un->transpose,
                                   (const GLfloat*)data);
                return;
//            case GL_INT:
//                glUniformMatrix2iv(un->location, un->count, un->transpose,
//                                   (const GLfloat*)data);
//                return;
//            case GL_UNSIGNED_INT:
//                glUniformMatrix2uiv(un->location, un->count, un->transpose,
//                                    (const GLfloat*)data);
//                return;
            default:
                printf("WARN: Invalid type for uniform value [%s]!\n", name);
                return;
            }
        case 2:
            glUniformMatrix2fv(un->location, un->count, un->transpose,
                               (const GLfloat*)data);
            return;
        case 3:
            glUniformMatrix2x3fv(un->location, un->count, un->transpose,
                                 (const GLfloat*)data);
            return;
        case 4:
            glUniformMatrix2x4fv(un->location, un->count, un->transpose,
                                 (const GLfloat*)data);
            return;
        default:
            printf("WARN: Invalid second matrix component: %d!\n", un->second);
            return;
        }
    case 3:
        switch( un->second ) {
        case 0:
        case 1:
            switch( un->type ) {
            case GL_FLOAT:
                glUniformMatrix3fv(un->location, un->count, un->transpose,
                                   (const GLfloat*)data);
                return;
//            case GL_INT:
//                glUniformMatrix3iv(un->location, un->count, un->transpose,
//                                   (const GLfloat*)data);
//                return;
//            case GL_UNSIGNED_INT:
//                glUniformMatrix3uiv(un->location, un->count, un->transpose,
//                                    (const GLfloat*)data);
//                return;
            default:
                printf("WARN: Invalid type for uniform value [%s]!\n", name);
                return;
            }
        case 2:
            glUniformMatrix3x2fv(un->location, un->count, un->transpose,
                                 (const GLfloat*)data);
            return;
        case 3:
            glUniformMatrix3fv(un->location, un->count, un->transpose,
                               (const GLfloat*)data);
            return;
        case 4:
            glUniformMatrix3x4fv(un->location, un->count, un->transpose,
                                 (const GLfloat*)data);
            return;
        default:
            printf("WARN: Invalid second matrix component: %d!\n", un->second);
            return;
        }
    case 4:
        switch( un->second ) {
        case 0:
        case 1:
            switch( un->type ) {
            case GL_FLOAT:
                glUniformMatrix4fv(un->location, un->count, un->transpose,
                                   (const GLfloat*)data);
                return;
//            case GL_INT:
//                glUniformMatrix4iv(un->location, un->count, un->transpose,
//                                   (const GLfloat*)data);
//                return;
//            case GL_UNSIGNED_INT:
//                glUniformMatrix4uiv(un->location, un->count, un->transpose,
//                                    (const GLfloat*)data);
//                return;
            default:
                printf("WARN: Invalid type for uniform value [%s]!\n", name);
                return;
            }
        case 2:
            glUniformMatrix4x2fv(un->location, un->count, un->transpose,
                                 (const GLfloat*)data);
            return;
        case 3:
            glUniformMatrix4x3fv(un->location, un->count, un->transpose,
                                 (const GLfloat*)data);
            return;
        case 4:
            glUniformMatrix4fv(un->location, un->count, un->transpose,
                               (const GLfloat*)data);
            return;
        default:
            printf("WARN: Invalid second matrix component: %d!\n", un->second);
            return;
        }
    default:
        printf("WARN: Invalid first matrix component: %d!\n", un->first);
        return;
    }
}

/*
 * Frees the given sstDrawableSet object, deleting with it all related OpenGL
 * objects.
 */
void sstFreeDrawableSet( sstDrawableSet *set ) {
    sstDrawable *d;
    /* Step 1: Delete OpenGL objects */
    glDeleteVertexArrays(1, &set->vao);
    for( d = set->drawables; d < set->drawables + set->size; d++ ) {
        glDeleteBuffers(1, &d->buffer);
    }
    /* Step 2: Free memory */
    free(set->drawables);
    free(set);
}

/*
 * Frees the given sstProgram object, deleting with it all related OpenGL
 * objects.
 */
void sstFreeProgram( sstProgram *program ) {
    int i;
    /* Step 1: Delete OpenGL objects */
    for( i = 0; i < program->shader_count; i++ ) {
        glDeleteShader(program->shaders[i]);
    }
    glDeleteProgram(program->program);
    /* Step 2: Free memory */
    free(program->inputs);
    free(program->uniforms);
    free(program->shaders);
    free(program);
}
