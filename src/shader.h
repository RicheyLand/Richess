#ifndef SHADER_H
#define SHADER_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/closest_point.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <map>
#include <vector>
#include <cmath>
#include <exception>
#include <stdexcept>
using namespace std;

class Shader                                            //  allows loading and compiling of both fragment and vertex shaders
{
private:
    void printShaderErrors(GLuint shader)               //  handle occurred compile errors of the shader
    {
        GLint success;
        GLchar message[1024];

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, message);    //  print loaded error message
            cerr << message << '\n';
        }
    }

    void printProgramErrors(GLuint shader)              //  handle occurred compile errors of the shader program
    {
        GLint success;
        GLchar message[1024];

        glGetProgramiv(shader, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, message);   //  print loaded error message
            cerr << message << '\n';
        }
    }

public:
    unsigned int ID;                                    //  unique identifier of the shader

    Shader(const char * vertexPath, const char * fragmentPath)  //  constructor requires paths to vertex and fragment shaders
    {
        string vertexCode = loadFile(vertexPath);       //  load file content using appropriate method
        string fragmentCode = loadFile(fragmentPath);

        const char * vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
                                                        //  compile both shaders
        unsigned int vertex, fragment;
                                                        //  handle vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        printShaderErrors(vertex);                      //  handle possible errors
                                                        //  handle fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        printShaderErrors(fragment);                    //  handle possible errors
        
                                                        //  handle shader Program
        ID = glCreateProgram();                         //  access identifier to the new shader program
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        printProgramErrors(ID);                         //  handle possible errors

        glDeleteShader(vertex);                         //  remove both shaders
        glDeleteShader(fragment);
    }

    string loadFile(const char * const file)            //  load whole file and return it as string
    {
        ifstream stream(file);
        if(stream.fail()) throw runtime_error(string("Can't open \'") + file + "\'");
        return string(istream_iterator<char>(stream >> noskipws), istream_iterator<char>());
    }
                                                        //  pass integer value into the shader
    void passInteger(const string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
                                                        //  pass vector into the shader
    void passVector(const string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
                                                        //  pass matrix into the shader
    void passMatrix(const string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void activate()                                     //  activate new shader
    { 
        glUseProgram(ID); 
    }
};

#endif
