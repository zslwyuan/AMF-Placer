
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