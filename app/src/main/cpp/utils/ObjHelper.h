//
// Created by mtdp on 2020-04-11.
//

#ifndef NATIVEACTIVITYDEMO_OBJHELPER_H
#define NATIVEACTIVITYDEMO_OBJHELPER_H

#include <cstdio>
#include <vector>
#include <unordered_map>
#include <GLES3/gl32.h>
#include "../entity/MapLocInfo.h"

class ObjHelper {
public:
    class ObjData {
    public:
        std::vector<GLfloat> minVertex = {0.0f, 0.0f, 0.0f}; // 物体中心为原点，包围盒的x，y，z最小值
        std::vector<GLfloat> maxVertex = {0.0f, 0.0f, 0.0f}; // 物体中心为原点，包围盒的x，y，z最大值

        std::vector<GLfloat> vertices; // 3个为一组
        std::vector<GLfloat> normals; // 3个为一组
        std::vector<GLfloat> texCoords; // 2个为一组
        std::vector<std::vector<GLushort>> indeces; // v vt vn的索引

        std::unordered_map<int, std::unordered_map<int, std::unique_ptr<MapLocInfo>>> mapLocInfos;
        ObjData() {
            vertices.push_back(0); // obj文件中，索引是从1开始的，这里先存入索引0的无用数据。
            vertices.push_back(0);
            vertices.push_back(0);

            normals.push_back(0);
            normals.push_back(0);
            normals.push_back(0);

            texCoords.push_back(0);
            texCoords.push_back(0);
        }
    };
    static float heightMapSampleFactor; // 表示取浮点数小数部分的位数，10表示1位，100表示两位等等。注意只能是整数。
    static void readObjFile(FILE *file, ObjData *pObjData, bool needGenMapInfo, bool hasTexCoords, bool isSmoothLight);
};

#endif //NATIVEACTIVITYDEMO_OBJHELPER_H
