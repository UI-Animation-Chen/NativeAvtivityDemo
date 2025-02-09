//
// Created by mtdp on 2020-04-11.
//

#include "ObjModel.h"
#include "../utils/ObjHelper.h"
#include "../utils/AndroidAssetUtils.h"
#include "../utils/CoordinatesUtils.h"
#include <cstring>
#include <cerrno>
#include "../utils/libglm0_9_6_3/glm/ext.hpp"

// 从free3d.com中下载.blender文件素材，导入blender后再导出为obj，导出设置：
// 1、z forward，y up；这种方式导出后x坐标是反的，ObjHelper.cpp中进行了处理。在视图和透视矩阵加入后，x坐标就不反了。
// 2、write normals, include uvs, triangulate faces，其他都不要选

ObjModel::ObjModel(const char *assetObjName, const char *assetPngName, bool needGenHeightMap,
                   bool hasTexCoords, bool isSmoothLight): Shape() {
    // assets目录下，文件后缀是png才能读到，否则会报错: no such file or directory.
    // 原因是：assets目录下的文件会进行压缩，所以读不到。而png会被认为是压缩文件，不会再次压缩。
//    const char *assetObjName = "blenderObjs/tower.png";
    int fd = AndroidAssetUtils::openFdFromAsset(assetObjName);
    if (fd <= 0) {
        app_log("openFdFromAsset \"%s\" failed: err: %s\n", assetObjName, strerror(errno));
        return;
    }
    FILE *file = fdopen(fd, "r");

    // 从手机sd读取
//    FILE *file = fopen("/sdcard/sphere.obj", "r");
//    if (file == NULL) {
//        app_log("file is NULL, err: %s\n", strerror(errno));
//        return;
//    }

    TextureUtils::loadPNGTexture(assetPngName, &textureId);

    auto pObjData = new ObjHelper::ObjData();
    ObjHelper::readObjFile(file, pObjData, needGenHeightMap, hasTexCoords, isSmoothLight);
    fclose(file);

    glGenVertexArrays(1, vao);
    glGenBuffers(4, buffers);

    // vertex data
    glBindVertexArray(vao[0]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    size_t verticesSize = sizeof(GLfloat) * pObjData->vertices.size();
    auto vertices = (GLfloat *)malloc(verticesSize);
    auto tmpVertices = vertices;
    for (GLfloat value: pObjData->vertices) {
        *tmpVertices++ = value;
    }
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);
    free(vertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // indeces
    indexCount = pObjData->indeces.size();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
    size_t indecesSize = sizeof(GLushort) * pObjData->indeces.size();
    auto indeces = (GLushort *)malloc(indecesSize);
    auto tmpIndeces = indeces;
    for (std::vector<GLushort> value: pObjData->indeces) {
        *tmpIndeces++ = value.at(0);
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indecesSize, indeces, GL_STATIC_DRAW);
    free(indeces);

    // texture coordinates data
    glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
    size_t texCoordsSize = sizeof(GLfloat) * pObjData->texCoords.size();
    auto texCoords = (GLfloat *)malloc(texCoordsSize);
    auto tmpTexCoords = texCoords;
    for (GLfloat value: pObjData->texCoords) {
        *tmpTexCoords++ = value;
    }
    glBufferData(GL_ARRAY_BUFFER, texCoordsSize, texCoords, GL_STATIC_DRAW);
    free(texCoords);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // normals data
    glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
    size_t normalsSize = sizeof(GLfloat) * pObjData->normals.size();
    auto normals = (GLfloat *)malloc(normalsSize);
    auto tmpNormals = normals;
    for (GLfloat value: pObjData->normals) {
        *tmpNormals++ = value;
    }
    glBufferData(GL_ARRAY_BUFFER, normalsSize, normals, GL_STATIC_DRAW);
    free(normals);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    mapLocInfos = std::move(pObjData->mapLocInfos);

    // 包围盒
    GLfloat minX = pObjData->minVertex.at(0);
    GLfloat minY = pObjData->minVertex.at(1);
    GLfloat minZ = pObjData->minVertex.at(2);
    GLfloat maxX = pObjData->maxVertex.at(0);
    GLfloat maxY = pObjData->maxVertex.at(1);
    GLfloat maxZ = pObjData->maxVertex.at(2);
    app_log("%s, min(x: %f, y: %f, z: %f), max(x: %f, y: %f, z: %f)\n", assetPngName, minX, minY, minZ, maxX, maxY, maxZ);
    initWrapBox(minX, minY, minZ, maxX, maxY, maxZ);

    if (needGenHeightMap) {
        CoordinatesUtils::insertLinearValue(mapLocInfos,
                                            (int)(minX * ObjHelper::heightMapSampleFactor),
                                            (int)(minZ * ObjHelper::heightMapSampleFactor),
                                            (int)(maxX * ObjHelper::heightMapSampleFactor),
                                            (int)(maxZ * ObjHelper::heightMapSampleFactor));
    }

    delete pObjData;

//    modelColorFactorV4[3] = 0.75f;
    glUniform3fv(lightPositionLocation, 1, lightPositionV3);
    glUniform3fv(lightColorLocation, 1, lightColorV3);
}

ObjModel::~ObjModel() {
    glDeleteVertexArrays(1, vao);
    glDeleteBuffers(4, buffers);
    glDeleteTextures(1, &textureId);
    app_log("ObjModel destructor~~~\n");
}

void ObjModel::draw() {
    Shape::draw();

    // obj
    glUniform1i(transformEnabledLocation, 1); // 开启shader中的transform
    modelColorFactorV4[3] = 1.0f;
    glUniform4fv(modelColorFactorLocation, 1, modelColorFactorV4);
    glBindTexture(GL_TEXTURE_2D, textureId); // img texture
    glBindVertexArray(vao[0]);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

    // 包围盒
    drawWrapBox3D();

    // wrapBox2D
    drawWrapBox2D();
}

GLfloat ObjModel::getMapHeight(GLfloat x, GLfloat z) {
    GLfloat scale[3] = {1.0f, 1.0f, 1.0f};
    getScale(scale);
    if (scale[0] == 0.0f) { // 防止除0异常
        scale[0] = 1.0f / 1000000.0f; // 随便给一个接近0的值，此时已经没意义了
    }
    if (scale[2] == 0.0f) { // 防止除0异常
        scale[2] = 1.0f / 1000000.0f;
    }
//    app_log("scale x: %f, z: %f, y: %f\n", scale[0], scale[2], scale[1]);
    int fixedX = (int)(x / scale[0] * ObjHelper::heightMapSampleFactor);
    int fixedZ = (int)(z / scale[2] * ObjHelper::heightMapSampleFactor);
    if (mapLocInfos.count(fixedX) != 0 && mapLocInfos[fixedX].count(fixedZ) != 0) {
        return mapLocInfos[fixedX][fixedZ]->height * scale[1];
    }
    return Shape::getMapHeight(fixedX, fixedZ); // alawys 0
}

void ObjModel::getMapNormal(GLfloat x, GLfloat z, glm::vec3 &outVec3) {
    GLfloat scale[3] = {1.0f, 1.0f, 1.0f};
    getScale(scale);
    if (scale[0] == 0.0f) { // 防止除0异常
        scale[0] = 1.0f / 1000000.0f; // 随便给一个接近0的值，此时已经没意义了
    }
    if (scale[2] == 0.0f) { // 防止除0异常
        scale[2] = 1.0f / 1000000.0f;
    }
    int fixedX = (int)(x / scale[0] * ObjHelper::heightMapSampleFactor);
    int fixedZ = (int)(z / scale[2] * ObjHelper::heightMapSampleFactor);
    if (mapLocInfos.count(fixedX) != 0 && mapLocInfos[fixedX].count(fixedZ) != 0) {
        outVec3[0] = mapLocInfos[fixedX][fixedZ]->normal[0] / scale[0]; // 该方向放大的倍数越大，法向量分量越小
        outVec3[1] = mapLocInfos[fixedX][fixedZ]->normal[1] / scale[1];
        outVec3[2] = mapLocInfos[fixedX][fixedZ]->normal[2] / scale[2];

        GLfloat rotate[3] = {0};
        getRotate(rotate);
        outVec3 = glm::rotate(outVec3, rotate[0], glm::vec3(1.0f, 0, 0)); // x
        outVec3 = glm::rotate(outVec3, rotate[1], glm::vec3(0, 1.0f, 0)); // y
        outVec3 = glm::rotate(outVec3, rotate[2], glm::vec3(0, 0, 1.0f)); // z
    }
}
