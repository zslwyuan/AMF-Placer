/**
 * @file GLDraw.h
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef _GLDRAW
#define _GLDRAW

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <math.h>
#include <string>
#include <vector>

int paintClusterNodeLine(std::vector<std::pair<float, float>> nodes, std::vector<std::pair<int, int>> lines,
                         const std::vector<std::vector<float>> &cluster2FixedUnitMat, std::vector<float> &_fixedX,
                         std::vector<float> &_fixedY);

int paintB2BNodeLine(std::vector<std::pair<float, float>> nodes, std::vector<std::pair<int, int>> lines);
#endif