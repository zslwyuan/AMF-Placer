#include "utils.h"
#include <util/torch.h>
#include "omp.h"
#include "simpleJSON.h"
#include "Rendering/paintDB.h"
#include "strPrint.h"

#include "Rendering/bl-qt-AMF.h"


void plotPlacement(PaintDataBase *paintData)
{
    char *argv[] = {"GUI"};
    int argc = 1;
    QApplication app(argc, argv);
    MainWindow win;
    win.paintData = paintData;
    win.setMinimumSize(QSize(400, 320));
    win.resize(QSize(WIN_W, WIN_H + 80));
    win.show();

    app.exec();
}

void runPlotThread(PaintDataBase *paintData)
{
    std::thread plot(plotPlacement, paintData);
    plot.detach();
}

void setOMPThread(int jobs)
{
    omp_set_num_threads(jobs);
}


PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
    pybind11::class_<PaintDataBase>(m, "PaintDataBase")
        .def(py::init<>());
    m.def("setOMPThread", &setOMPThread);
    m.def("parseJSONFile", &parseJSONFile);
    m.def("print_status", &print_status);
    m.def("print_info", &print_info);
    m.def("print_error", &print_error);
    m.def("plotPlacement", &plotPlacement);
    m.def("runPlotThread", &runPlotThread);
     // FFSRCompatible
    //m.def("print", &print, "compute pin RUDY map");
}