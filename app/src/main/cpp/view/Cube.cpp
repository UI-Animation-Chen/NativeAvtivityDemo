//
// Created by czf on 2019-10-23.
//

#include <GLES3/gl32.h>

#include "../gles/ShaderUtils.h"
#include "Cube.h"
#include "../utils/CoordinatesUtils.h"

static const char *cubeVert = "#version 300 es\n"
                              "layout(location = 1) in vec4 vPositionCube;\n"
                              "uniform vec4 positionOffset;\n"
                              "void main() {\n"
                              "  gl_Position = vPositionCube + positionOffset;\n"
                              "}\n";

static const char *cubeFrag = "#version 300 es\n"
                              "precision mediump float;\n"
                              "out vec4 fColorCube;\n"
                              "void main() {\n"
                              "  fColorCube = vec4(1.0, 1.0, 1.0, 1.0);\n"
                              "}\n";

Cube::Cube() {
  init_shaders();

  glGenVertexArrays(1, &vao);
  glGenBuffers(2, buffers);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
//  GLfloat cubePoints[] = {
//      -0.5f, -0.5f, 0.0f,
//      0.5f, -0.5f, 0.0f,
//      -0.5f,  0.5f, 0.0f,
//      0.5f, 0.5f, 0.0f
//  };
  GLfloat cubePoints[] = {
    0.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    0.5f,  -0.5f, 0.0f,
    0.5f, 0.5f, 0.0f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubePoints), cubePoints, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
  GLushort cubeIndices[] = {
    0, 1, 2, 0, 2, 3, 0, 3, 4
  };
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
}

Cube::~Cube() {
  glDeleteBuffers(2, buffers);
  glDeleteVertexArrays(1, &vao);

  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
  glDeleteProgram(program);
}

void Cube::move(float offsetX, float offsetY, float offsetZ) {
  GLfloat offsets[4] = {0};
  offsets[0] = CoordinatesUtils::android2gles_x(offsetX);
  offsets[0] = 0.1f;
  offsets[1] = CoordinatesUtils::android2gles_x(offsetY);
  offsets[1] = 0.1f;
  offsets[2] = CoordinatesUtils::android2gles_x(offsetZ);
  glUniform4fv(offsetLocation, 4, offsets);
}

void Cube::draw() {
  // 1->2->3, 2->3->4，先逆时针，后顺时针。
//  glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
  glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_SHORT, 0);
}

void Cube::init_shaders() {
  vertShader = get_compiled_shader_vert(cubeVert);
  fragShader = get_compiled_shader_frag(cubeFrag);
  program = linkShader(vertShader, fragShader);
  glUseProgram(program);
  offsetLocation = glGetUniformLocation(program, "positionOffset");
}
