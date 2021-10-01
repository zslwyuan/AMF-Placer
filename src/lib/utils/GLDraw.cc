/**
 * @file GLDraw.cc
 * @author Tingyuan LIANG (tliang@connect.ust.hk)
 * @brief
 * @version 0.1
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "GLDraw.h"
#include <iostream>
#include <unistd.h>
#include <new>
const GLfloat Pi = 3.1415926536f;

float gl_random_float(float min, float max)
{
    return ((float)rand() / RAND_MAX) * (max - min) + min;
}

int glut_loop_continue = 1;
enum EVA
{
    EVA_triangle,
    EVA_Num
};
GLuint aVAs[EVA_Num];
enum EB
{
    EB_triangle,
    EB_NUM
};
GLuint aBufs[EB_NUM];

std::vector<std::pair<float, float>> GLAnchorNodes;
std::vector<std::pair<float, float>> GLNodeCenterXY;
std::vector<std::pair<int, int>> GLLinesAmongCluster;
std::vector<std::pair<int, int>> GLLinesBetweenClusterFixed;
std::vector<float> fixedX;
std::vector<float> fixedY;

int mouseX = 100, mouseY = 100, wheeldir = 0;
GLfloat (*nodeDrawBuf)[2];
std::vector<std::pair<float, float>> leftright_range;
std::vector<std::pair<float, float>> bottomtop_range;
float zoomInScale = 1.1;
int curWindowWidth, curWindowHeight;

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glLoadIdentity();
    curWindowWidth = width;
    curWindowHeight = height;
    glOrtho(leftright_range[leftright_range.size() - 1].first, leftright_range[leftright_range.size() - 1].second,
            bottomtop_range[bottomtop_range.size() - 1].first, bottomtop_range[bottomtop_range.size() - 1].second,
            -10.0, 10.0);
}

void key(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // Esc
        // glutLeaveMainLoop();
        glut_loop_continue = 0;
        break;
    case 'i':
    {
        float realX = leftright_range[leftright_range.size() - 1].first +
                      (float)mouseX / (float)curWindowWidth *
                          (leftright_range[leftright_range.size() - 1].second -
                           leftright_range[leftright_range.size() - 1].first);
        float realY =
            bottomtop_range[bottomtop_range.size() - 1].first +
            (1 - (float)mouseY / (float)curWindowHeight) * (bottomtop_range[bottomtop_range.size() - 1].second -
                                                            bottomtop_range[bottomtop_range.size() - 1].first);
        // std::cout << "zoom in at: " << realX << " " << realY << "\n" << std::flush;
        float half_newwidth =
            (leftright_range[leftright_range.size() - 1].second - leftright_range[leftright_range.size() - 1].first) /
            zoomInScale / 2;
        float half_newheight =
            (bottomtop_range[bottomtop_range.size() - 1].second - bottomtop_range[bottomtop_range.size() - 1].first) /
            zoomInScale / 2;
        leftright_range.push_back(std::pair<float, float>(realX - half_newwidth, realX + half_newwidth));
        bottomtop_range.push_back(std::pair<float, float>(realY - half_newheight, realY + half_newheight));
        break;
    }
    case 'I':
    {

        float realX = leftright_range[leftright_range.size() - 1].first +
                      (float)mouseX / (float)curWindowWidth *
                          (leftright_range[leftright_range.size() - 1].second -
                           leftright_range[leftright_range.size() - 1].first);
        float realY =
            bottomtop_range[bottomtop_range.size() - 1].first +
            (1 - (float)mouseY / (float)curWindowHeight) * (bottomtop_range[bottomtop_range.size() - 1].second -
                                                            bottomtop_range[bottomtop_range.size() - 1].first);
        // std::cout << "zoom in at: " << realX << " " << realY << "\n" << std::flush;
        float half_newwidth =
            (leftright_range[leftright_range.size() - 1].second - leftright_range[leftright_range.size() - 1].first) /
            zoomInScale / 2;
        float half_newheight =
            (bottomtop_range[bottomtop_range.size() - 1].second - bottomtop_range[bottomtop_range.size() - 1].first) /
            zoomInScale / 2;
        leftright_range.push_back(std::pair<float, float>(realX - half_newwidth, realX + half_newwidth));
        bottomtop_range.push_back(std::pair<float, float>(realY - half_newheight, realY + half_newheight));
        break;
    }
    case 'o':
    {
        // std::cout << "zoom out\n" << std::flush;
        if (leftright_range.size() > 1)
        {
            leftright_range.pop_back();
            bottomtop_range.pop_back();
        }
        else
        {
            std::cout << "cannot further zoom out\n" << std::flush;
        }
        break;
    }
    case 'O':
    {
        // std::cout << "zoom out\n" << std::flush;
        if (leftright_range.size() > 1)
        {
            leftright_range.pop_back();
            bottomtop_range.pop_back();
        }
        else
        {
            std::cout << "cannot further zoom out\n" << std::flush;
        }
        break;
    }
    }
    glLoadIdentity();
    glOrtho(leftright_range[leftright_range.size() - 1].first, leftright_range[leftright_range.size() - 1].second,
            bottomtop_range[bottomtop_range.size() - 1].first, bottomtop_range[bottomtop_range.size() - 1].second,
            -10.0, 10.0);
}

void idle()
{
    if (1)
    {
        glutPostRedisplay();
    }
}

void releaseClusterRC()
{
    glDeleteVertexArrays(EVA_Num, aVAs);
    glDeleteBuffers(EB_NUM, aBufs);
}

void displayClusterNodeLine()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(aVAs[EVA_triangle]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, aBufs[EB_triangle]);

    glColor3f(1.0f, 1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, GLAnchorNodes.size() * 6);
    glColor3f(0.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINES, GLAnchorNodes.size() * 6, GLLinesAmongCluster.size() * 2);
    glColor3f(1.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, GLAnchorNodes.size() * 6 + GLLinesAmongCluster.size() * 2,
                 (GLLinesBetweenClusterFixed.size()) * 2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glutSwapBuffers();
}

void initClusterRC()
{
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glPointSize(2.0f);
    glLineWidth(2.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glGenVertexArrays(EVA_Num, aVAs);
    glBindVertexArray(aVAs[EVA_triangle]);

    glGenBuffers(EB_NUM, aBufs);
    glBindBuffer(GL_ARRAY_BUFFER, aBufs[EB_triangle]);

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * GLAnchorNodes.size() * 12 +
                     sizeof(GLfloat) * (GLLinesAmongCluster.size() + GLLinesBetweenClusterFixed.size()) * 4,
                 nodeDrawBuf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setClusterNodeLine()
{
    nodeDrawBuf =
        new GLfloat[GLAnchorNodes.size() * 6 + (GLLinesAmongCluster.size() + GLLinesBetweenClusterFixed.size()) * 2][2];
    GLNodeCenterXY.resize(GLAnchorNodes.size());
    for (unsigned int i = 0; i < GLAnchorNodes.size(); i++)
    {
        auto node = GLAnchorNodes[i];
        float rectW = 20;
        float rectH = 20;
        float cx = node.first + gl_random_float(-3, 3);
        float cy = node.second + gl_random_float(-3, 3);
        GLNodeCenterXY[i] = std::pair<float, float>(cx * 10, cy * 10);
        float x0, x1, x2, x3, y0, y1, y2, y3;
        x0 = cx * 10 - rectW / 2;
        y0 = cy * 10 - rectH / 2;
        x1 = cx * 10 + rectW / 2;
        y1 = cy * 10 - rectH / 2;
        x2 = cx * 10 + rectW / 2;
        y2 = cy * 10 + rectH / 2;
        x3 = cx * 10 - rectW / 2;
        y3 = cy * 10 + rectH / 2;
        nodeDrawBuf[i * 6][0] = x0;
        nodeDrawBuf[i * 6][1] = y0;
        nodeDrawBuf[i * 6 + 1][0] = x1;
        nodeDrawBuf[i * 6 + 1][1] = y1;
        nodeDrawBuf[i * 6 + 2][0] = x2;
        nodeDrawBuf[i * 6 + 2][1] = y2;
        nodeDrawBuf[i * 6 + 3][0] = x2;
        nodeDrawBuf[i * 6 + 3][1] = y2;
        nodeDrawBuf[i * 6 + 4][0] = x3;
        nodeDrawBuf[i * 6 + 4][1] = y3;
        nodeDrawBuf[i * 6 + 5][0] = x0;
        nodeDrawBuf[i * 6 + 5][1] = y0;
    }
    for (unsigned int i = 0; i < GLLinesAmongCluster.size(); i++)
    {
        int idA = GLLinesAmongCluster[i].first, idB = GLLinesAmongCluster[i].second;
        float cx0 = GLNodeCenterXY[idA].first;
        float cy0 = GLNodeCenterXY[idA].second;
        float cx1 = GLNodeCenterXY[idB].first;
        float cy1 = GLNodeCenterXY[idB].second;
        nodeDrawBuf[GLAnchorNodes.size() * 6 + i * 2][0] = cx0;
        nodeDrawBuf[GLAnchorNodes.size() * 6 + i * 2][1] = cy0;
        nodeDrawBuf[GLAnchorNodes.size() * 6 + i * 2 + 1][0] = cx1;
        nodeDrawBuf[GLAnchorNodes.size() * 6 + i * 2 + 1][1] = cy1;
        // std::cout << idA << "-" << idB << ": " << cx0 << " " << cy0 << " " << cx1 << " " << cy1 << "\n";
    }

    int nodeOffset = GLAnchorNodes.size() * 6 + GLLinesAmongCluster.size() * 2;
    for (unsigned int i = 0; i < GLLinesBetweenClusterFixed.size(); i++)
    {
        int idA = GLLinesBetweenClusterFixed[i].first, idB = GLLinesBetweenClusterFixed[i].second;
        float cx0 = GLNodeCenterXY[idA].first;
        float cy0 = GLNodeCenterXY[idA].second;
        float cx1 = fixedX[idB] * 10;
        float cy1 = fixedY[idB] * 10;
        nodeDrawBuf[nodeOffset + i * 2][0] = cx0;
        nodeDrawBuf[nodeOffset + i * 2][1] = cy0;
        nodeDrawBuf[nodeOffset + i * 2 + 1][0] = cx1;
        nodeDrawBuf[nodeOffset + i * 2 + 1][1] = cy1;
        // std::cout << idA << "-" << idB << ": " << cx0 << " " << cy0 << " " << cx1 << " " << cy1 << "\n";
    }
}

int paintClusterNodeLine(std::vector<std::pair<float, float>> nodes, std::vector<std::pair<int, int>> lines,
                         const std::vector<std::vector<float>> &cluster2FixedUnitMat, std::vector<float> &_fixedX,
                         std::vector<float> &_fixedY)
{
#pragma GCC diagnostic ignored "-Wwrite-strings"
    leftright_range.clear();
    bottomtop_range.clear();
    leftright_range.push_back(std::pair<float, float>(-100, 900));
    bottomtop_range.push_back(std::pair<float, float>(0, 4800));
    char *argv[] = {"ClusterPlacementPainter"};
    int argc = 1;
    GLAnchorNodes = nodes;
    GLLinesAmongCluster = lines;
    fixedX = _fixedX;
    fixedY = _fixedY;
    GLLinesBetweenClusterFixed.clear();
    for (unsigned int clusterId = 0; clusterId < cluster2FixedUnitMat.size(); clusterId++)
        for (unsigned int fixedId = 0; fixedId < cluster2FixedUnitMat[clusterId].size(); fixedId++)
        {
            if (cluster2FixedUnitMat[clusterId][fixedId] > 0)
            {
                GLLinesBetweenClusterFixed.push_back(std::pair<int, int>(clusterId, fixedId));
            }
        }

    setClusterNodeLine();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(333, 2000);

    int mainwnd = glutCreateWindow("glut window");

    glutReshapeFunc(reshape);
    glutDisplayFunc(displayClusterNodeLine);
    glutKeyboardFunc(key);

    if (glewInit() != GLEW_OK)
    {
        return -1;
    }

    initClusterRC();

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glut_loop_continue = true;
    while (glut_loop_continue)
    {
        if (glutGetWindow() == 0)
            break;
        glutMainLoopEvent();
        idle();
        sleep(1);
    }

    releaseClusterRC();
    delete[] nodeDrawBuf;
    glutDestroyWindow(mainwnd);
    glutExit();
    return 0;
}

void releaseB2BRC()
{
    glDeleteVertexArrays(EVA_Num, aVAs);
    glDeleteBuffers(EB_NUM, aBufs);
}

void displayB2BNodeLine()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(aVAs[EVA_triangle]);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, aBufs[EB_triangle]);

    glColor3f(1.0f, 1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, GLAnchorNodes.size() * 6);
    glColor3f(0.0f, 1.0f, 1.0f);
    glDrawArrays(GL_LINES, GLAnchorNodes.size() * 6, GLLinesAmongCluster.size() * 2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glutSwapBuffers();
}

void initB2BRC()
{
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glPointSize(2.0f);
    glLineWidth(2.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glGenVertexArrays(EVA_Num, aVAs);
    glBindVertexArray(aVAs[EVA_triangle]);

    glGenBuffers(EB_NUM, aBufs);
    glBindBuffer(GL_ARRAY_BUFFER, aBufs[EB_triangle]);

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * GLAnchorNodes.size() * 12 + sizeof(GLfloat) * (GLLinesAmongCluster.size()) * 4,
                 nodeDrawBuf, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setB2BNodeLine()
{
    nodeDrawBuf =
        new GLfloat[GLAnchorNodes.size() * 6 + (GLLinesAmongCluster.size() + GLLinesBetweenClusterFixed.size()) * 2][2];
    GLNodeCenterXY.resize(GLAnchorNodes.size());
    for (unsigned int i = 0; i < GLAnchorNodes.size(); i++)
    {
        auto node = GLAnchorNodes[i];
        float rectW = 10;
        float rectH = 10;
        float cx = node.first;
        float cy = node.second;
        GLNodeCenterXY[i] = std::pair<float, float>(cx * 10, cy * 10);
        float x0, x1, x2, x3, y0, y1, y2, y3;
        x0 = cx * 10 - rectW / 2;
        y0 = cy * 10 - rectH / 2;
        x1 = cx * 10 + rectW / 2;
        y1 = cy * 10 - rectH / 2;
        x2 = cx * 10 + rectW / 2;
        y2 = cy * 10 + rectH / 2;
        x3 = cx * 10 - rectW / 2;
        y3 = cy * 10 + rectH / 2;
        nodeDrawBuf[i * 6][0] = x0;
        nodeDrawBuf[i * 6][1] = y0;
        nodeDrawBuf[i * 6 + 1][0] = x1;
        nodeDrawBuf[i * 6 + 1][1] = y1;
        nodeDrawBuf[i * 6 + 2][0] = x2;
        nodeDrawBuf[i * 6 + 2][1] = y2;
        nodeDrawBuf[i * 6 + 3][0] = x2;
        nodeDrawBuf[i * 6 + 3][1] = y2;
        nodeDrawBuf[i * 6 + 4][0] = x3;
        nodeDrawBuf[i * 6 + 4][1] = y3;
        nodeDrawBuf[i * 6 + 5][0] = x0;
        nodeDrawBuf[i * 6 + 5][1] = y0;
    }
    for (unsigned int i = 0; i < GLLinesAmongCluster.size(); i++)
    {
        int idA = GLLinesAmongCluster[i].first, idB = GLLinesAmongCluster[i].second;
        float cx0 = GLNodeCenterXY[idA].first;
        float cy0 = GLNodeCenterXY[idA].second;
        float cx1 = GLNodeCenterXY[idB].first;
        float cy1 = GLNodeCenterXY[idB].second;
        nodeDrawBuf[GLAnchorNodes.size() * 6 + i * 2][0] = cx0;
        nodeDrawBuf[GLAnchorNodes.size() * 6 + i * 2][1] = cy0;
        nodeDrawBuf[GLAnchorNodes.size() * 6 + i * 2 + 1][0] = cx1;
        nodeDrawBuf[GLAnchorNodes.size() * 6 + i * 2 + 1][1] = cy1;
        // std::cout << idA << "-" << idB << ": " << cx0 << " " << cy0 << " " << cx1 << " " << cy1 << "\n";
    }
}

void mouseWheel(int button, int dir, int x, int y)
{
    if (dir > 0)
    {
        wheeldir = dir;
        // Zoom in
    }
    else
    {
        wheeldir = dir;
        // Zoom out
    }
    glutPostRedisplay();
    return;
}

void passiveMouse(int x1, int y1)
{
    mouseX = x1;
    mouseY = y1;
}

int paintB2BNodeLine(std::vector<std::pair<float, float>> nodes, std::vector<std::pair<int, int>> lines)
{

    leftright_range.clear();
    bottomtop_range.clear();
    leftright_range.push_back(std::pair<float, float>(-100, 900));
    bottomtop_range.push_back(std::pair<float, float>(0, 4800));
#pragma GCC diagnostic ignored "-Wwrite-strings"
    char *argv[] = {"B2BNetPainter"};
    int argc = 1;
    GLAnchorNodes = nodes;
    GLLinesAmongCluster = lines;
    curWindowWidth = 333;
    curWindowHeight = 2000;
    setB2BNodeLine();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(333, 2000);

    int mainwnd = glutCreateWindow("glut window");

    glutReshapeFunc(reshape);
    glutDisplayFunc(displayB2BNodeLine);
    glutKeyboardFunc(key);
    glutMouseWheelFunc(mouseWheel);
    glutPassiveMotionFunc(passiveMouse);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "failed to initialized\n" << std::flush;
        return -1;
    }

    initB2BRC();

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glut_loop_continue = true;
    while (glut_loop_continue)
    {
        if (glutGetWindow() == 0)
        {
            std::cout << "failed to find window\n" << std::flush;
            break;
        }
        glutMainLoopEvent();
        idle();
        sleep(1);
        wheeldir = 0;
    }

    releaseB2BRC();
    delete[] nodeDrawBuf;
    glutDestroyWindow(mainwnd);
    glutExit();
    return 0;
}
