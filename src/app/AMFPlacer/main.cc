/**
 * @file main.cc
 * @author Tingyuan Liang (tliang@connect.ust.hk)
 * @brief AMF-Placer Main file which directly pass the arguments to the AMFPacer workflow
 * @version 0.1
 * @date 2021-05-31
 *
 * @copyright Copyright (c) 2021 Reconfiguration Computing Systems Lab, The Hong Kong University of Science and
 * Technology. All rights reserved.
 *
 */

#include "3rdParty/Rendering/bl-qt-AMF.h"
#include "AMFPlacer.h"
// #include "3rdParty/pybind11/include/pybind11/pybind11.h"

void runPlacer(AMFPlacer *placer)
{
    placer->run();
}

void runVisualization(AMFPlacer *placer)
{
    char *argv[] = {"GUI"};
    int argc = 1;
    QApplication app(argc, argv);
    MainWindow win;
    win.paintData = placer->paintData;
    win.setMinimumSize(QSize(400, 320));
    win.resize(QSize(WIN_W, WIN_H + 80));
    win.show();

    app.exec();
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config JSON file> [-gui]" << std::endl;
        return 1;
    }
    bool guiEnable = false;
    if (argc == 3)
    {
        std::string arg2(argv[2]);
        if (arg2 == "-gui")
            guiEnable = true;
    }

    AMFPlacer *placer = new AMFPlacer(argv[1], guiEnable);

    std::thread threadPlacer(runPlacer, placer);
    while (!placer->paintData)
    {
    };
    std::thread *threadPaint = nullptr;
    if (guiEnable)
        threadPaint = new std::thread(runVisualization, placer);
    threadPlacer.join();
    if (threadPaint)
        threadPaint->join();

    delete placer;

    return 0;
}
