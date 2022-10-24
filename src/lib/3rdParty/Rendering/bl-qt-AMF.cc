#include "bl-qt-AMF.h"
#include "bl-qt-static.h"
#include "qblcanvas.h"

#include <stdlib.h>
#include <vector>

static QPainter::CompositionMode Blend2DCompOpToQtCompositionMode(BLCompOp compOp)
{
    switch (compOp)
    {
    default:
    case BL_COMP_OP_SRC_OVER:
        return QPainter::CompositionMode_SourceOver;
    case BL_COMP_OP_SRC_COPY:
        return QPainter::CompositionMode_Source;
    case BL_COMP_OP_SRC_IN:
        return QPainter::CompositionMode_SourceIn;
    case BL_COMP_OP_SRC_OUT:
        return QPainter::CompositionMode_SourceOut;
    case BL_COMP_OP_SRC_ATOP:
        return QPainter::CompositionMode_SourceAtop;
    case BL_COMP_OP_DST_OVER:
        return QPainter::CompositionMode_DestinationOver;
    case BL_COMP_OP_DST_COPY:
        return QPainter::CompositionMode_Destination;
    case BL_COMP_OP_DST_IN:
        return QPainter::CompositionMode_DestinationIn;
    case BL_COMP_OP_DST_OUT:
        return QPainter::CompositionMode_DestinationOut;
    case BL_COMP_OP_DST_ATOP:
        return QPainter::CompositionMode_DestinationAtop;
    case BL_COMP_OP_XOR:
        return QPainter::CompositionMode_Xor;
    case BL_COMP_OP_CLEAR:
        return QPainter::CompositionMode_Clear;
    case BL_COMP_OP_PLUS:
        return QPainter::CompositionMode_Plus;
    case BL_COMP_OP_MULTIPLY:
        return QPainter::CompositionMode_Multiply;
    case BL_COMP_OP_SCREEN:
        return QPainter::CompositionMode_Screen;
    case BL_COMP_OP_OVERLAY:
        return QPainter::CompositionMode_Overlay;
    case BL_COMP_OP_DARKEN:
        return QPainter::CompositionMode_Darken;
    case BL_COMP_OP_LIGHTEN:
        return QPainter::CompositionMode_Lighten;
    case BL_COMP_OP_COLOR_DODGE:
        return QPainter::CompositionMode_ColorDodge;
    case BL_COMP_OP_COLOR_BURN:
        return QPainter::CompositionMode_ColorBurn;
    case BL_COMP_OP_HARD_LIGHT:
        return QPainter::CompositionMode_HardLight;
    case BL_COMP_OP_SOFT_LIGHT:
        return QPainter::CompositionMode_SoftLight;
    case BL_COMP_OP_DIFFERENCE:
        return QPainter::CompositionMode_Difference;
    case BL_COMP_OP_EXCLUSION:
        return QPainter::CompositionMode_Exclusion;
    }
}

MainWindow::MainWindow() : _random(0x1234), _compOp(BL_COMP_OP_SRC_OVER), _shapeType(0), _rectSize(64.0)
{
    // LUT,FF,MUX,CARRY,DSP,BRAM,LUTRAM
    // float _type2W[10] = {1, 1, 1, 1, 2, 2, 1};
    // float _type2H[10] = {1, 1, 1, 2, 5, 7.5, 2};
    _type2C.resize(9);
    _type2C[0] = BLRgba32(0xFF00FFFF); // LUT
    _type2C[1] = BLRgba32(0xFFFF0000); // FF
    _type2C[2] = BLRgba32(0xFF00FF00); // MUX
    _type2C[3] = BLRgba32(0xFFFFFF00); // CARRY
    _type2C[4] = BLRgba32(0xFFFF00FF); // DSP
    _type2C[5] = BLRgba32(0xFFFFFFFF); // BRAM
    _type2C[6] = BLRgba32(0xFF2E2EFF); // LUTRAM
    _type2C[7] = BLRgba32(0xA4ECFF83); // Blockage
    _type2C[8] = BLRgba32(0xFFFFFFFF); // Blockage

    QVBoxLayout *vBox = new QVBoxLayout();
    vBox->setContentsMargins(0, 0, 0, 0);
    vBox->setSpacing(0);

    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(5, 5, 5, 5);
    grid->setSpacing(5);

    QBLCanvas::initRendererSelectBox(&_rendererSelect);
    _compOpSelect.addItem("SrcOver", QVariant(int(BL_COMP_OP_SRC_OVER)));
    _compOpSelect.addItem("SrcCopy", QVariant(int(BL_COMP_OP_SRC_COPY)));
    _compOpSelect.addItem("DstAtop", QVariant(int(BL_COMP_OP_DST_ATOP)));
    _compOpSelect.addItem("Xor", QVariant(int(BL_COMP_OP_XOR)));
    _compOpSelect.addItem("Plus", QVariant(int(BL_COMP_OP_PLUS)));
    _compOpSelect.addItem("Screen", QVariant(int(BL_COMP_OP_SCREEN)));
    _compOpSelect.addItem("Lighten", QVariant(int(BL_COMP_OP_LIGHTEN)));
    _compOpSelect.addItem("Hard Light", QVariant(int(BL_COMP_OP_HARD_LIGHT)));
    _compOpSelect.addItem("Difference", QVariant(int(BL_COMP_OP_DIFFERENCE)));

    _shapeTypeSelect.addItem("Rect", QVariant(int(kShapeRect)));

    _limitFpsCheck.setText(QLatin1String("Limit FPS"));

    _sizeSlider.setOrientation(Qt::Horizontal);
    _sizeSlider.setMinimum(8);
    _sizeSlider.setMaximum(128);
    _sizeSlider.setSliderPosition(64);

    _countSlider.setOrientation(Qt::Horizontal);
    _countSlider.setMinimum(1);
    _countSlider.setMaximum(10000);
    _countSlider.setSliderPosition(200);

    _canvas.onRenderB2D = std::bind(&MainWindow::onRenderB2D, this, std::placeholders::_1);
    _canvas.onRenderQt = std::bind(&MainWindow::onRenderQt, this, std::placeholders::_1);
    _canvas.onMouseEvent = std::bind(&MainWindow::onMouseEvent, this, std::placeholders::_1);

    connect(&_rendererSelect, SIGNAL(activated(int)), SLOT(onRendererChanged(int)));
    connect(&_compOpSelect, SIGNAL(activated(int)), SLOT(onCompOpChanged(int)));
    connect(&_shapeTypeSelect, SIGNAL(activated(int)), SLOT(onShapeTypeChanged(int)));
    connect(&_limitFpsCheck, SIGNAL(stateChanged(int)), SLOT(onLimitFpsChanged(int)));
    connect(&_sizeSlider, SIGNAL(valueChanged(int)), SLOT(onSizeChanged(int)));
    connect(&_countSlider, SIGNAL(valueChanged(int)), SLOT(onCountChanged(int)));

    grid->addWidget(new QLabel("Renderer:"), 0, 0);
    grid->addWidget(&_rendererSelect, 0, 1);

    grid->addWidget(new QLabel("Comp Op:"), 0, 2);
    grid->addWidget(&_compOpSelect, 0, 3);

    grid->addWidget(new QLabel("Shape:"), 0, 4);
    grid->addWidget(&_shapeTypeSelect, 0, 5);

    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 6);
    grid->addWidget(&_limitFpsCheck, 0, 7, Qt::AlignRight);

    grid->addWidget(new QLabel("Count:"), 1, 0, 1, 1, Qt::AlignRight);
    grid->addWidget(&_countSlider, 1, 1, 1, 7);

    grid->addWidget(new QLabel("Size:"), 2, 0, 1, 1, Qt::AlignRight);
    grid->addWidget(&_sizeSlider, 2, 1, 1, 7);

    vBox->addLayout(grid);
    vBox->addWidget(&_canvas);
    setLayout(vBox);

    connect(&_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    onInit();
}

// int main(int argc, char *argv[]) {
//   QApplication app(argc, argv);
//   MainWindow win;

//   win.setMinimumSize(QSize(400, 320));
//   win.resize(QSize(580, 520));
//   win.show();

//   return app.exec();
// }
#include "bl-qt-AMF.moc"