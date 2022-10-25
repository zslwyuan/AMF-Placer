#include "bl-qt-AMF.h"
#include "bl-qt-static.h"
#include "qblcanvas.h"

#include <stdlib.h>
#include <vector>

#include <QStyleOptionSlider>
#include <QToolTip>

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
    setFocusPolicy(Qt::StrongFocus);
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

    _compOpSelect.addItem("SrcOver", QVariant(int(BL_COMP_OP_SRC_OVER)));
    _compOpSelect.addItem("SrcCopy", QVariant(int(BL_COMP_OP_SRC_COPY)));
    _compOpSelect.addItem("DstAtop", QVariant(int(BL_COMP_OP_DST_ATOP)));
    _compOpSelect.addItem("Xor", QVariant(int(BL_COMP_OP_XOR)));
    _compOpSelect.addItem("Plus", QVariant(int(BL_COMP_OP_PLUS)));
    _compOpSelect.addItem("Screen", QVariant(int(BL_COMP_OP_SCREEN)));
    _compOpSelect.addItem("Lighten", QVariant(int(BL_COMP_OP_LIGHTEN)));
    _compOpSelect.addItem("Hard Light", QVariant(int(BL_COMP_OP_HARD_LIGHT)));
    _compOpSelect.addItem("Difference", QVariant(int(BL_COMP_OP_DIFFERENCE)));

    _LUTCheck.setText(QLatin1String("LUT"));
    _FFCheck.setText(QLatin1String("FF"));
    _MUXCheck.setText(QLatin1String("MUX"));
    _CARRYCheck.setText(QLatin1String("CARRY"));
    _DSPCheck.setText(QLatin1String("DSP"));
    _BRAMCheck.setText(QLatin1String("BRAM"));
    _OtherCheck.setText(QLatin1String("Others"));

    _sizeSlider.setOrientation(Qt::Horizontal);
    _sizeSlider.setMinimum(8);
    _sizeSlider.setMaximum(128);
    _sizeSlider.setSliderPosition(64);

    _countSlider.setOrientation(Qt::Horizontal);
    _countSlider.setMinimum(1);
    _countSlider.setMaximum(100);
    _countSlider.setSliderPosition(1);

    _canvas.onRenderB2D = std::bind(&MainWindow::onRenderB2D, this, std::placeholders::_1);
    _canvas.onMouseEvent = std::bind(&MainWindow::onMouseEvent, this, std::placeholders::_1);

    _canvas.onKeyPressEvent = std::bind(&MainWindow::keyPressEvent, this, std::placeholders::_1);
    _canvas.onKeyReleaseEvent = std::bind(&MainWindow::keyReleaseEvent, this, std::placeholders::_1);

    connect(&_compOpSelect, SIGNAL(activated(int)), SLOT(onCompOpChanged(int)));
    connect(&_sizeSlider, SIGNAL(valueChanged(int)), SLOT(onSizeChanged(int)));
    connect(&_countSlider, SIGNAL(valueChanged(int)), SLOT(onCountChanged(int)));

    connect(&_LUTCheck, SIGNAL(stateChanged(int)), SLOT(onLUTChanged(int)));
    connect(&_FFCheck, SIGNAL(stateChanged(int)), SLOT(onFFChanged(int)));
    connect(&_MUXCheck, SIGNAL(stateChanged(int)), SLOT(onMUXChanged(int)));
    connect(&_CARRYCheck, SIGNAL(stateChanged(int)), SLOT(onCARRYChanged(int)));
    connect(&_DSPCheck, SIGNAL(stateChanged(int)), SLOT(onDSPChanged(int)));
    connect(&_BRAMCheck, SIGNAL(stateChanged(int)), SLOT(onBRAMChanged(int)));
    connect(&_OtherCheck, SIGNAL(stateChanged(int)), SLOT(onOtherChanged(int)));

    grid->addWidget(new QLabel("ColorStyle:"), 0, 2);
    grid->addWidget(&_compOpSelect, 0, 3);

    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 0, 6);

    grid->addWidget(new QLabel("#CriticalPath:"), 1, 0, 1, 1, Qt::AlignRight);
    grid->addWidget(&_countSlider, 1, 1, 1, 7);

    grid->addWidget(&_LUTCheck, 2, 0, Qt::AlignRight);
    grid->addWidget(&_FFCheck, 2, 1, Qt::AlignRight);
    grid->addWidget(&_MUXCheck, 2, 2, Qt::AlignRight);
    grid->addWidget(&_CARRYCheck, 2, 3, Qt::AlignRight);
    grid->addWidget(&_DSPCheck, 2, 4, Qt::AlignRight);
    grid->addWidget(&_BRAMCheck, 2, 5, Qt::AlignRight);
    grid->addWidget(&_OtherCheck, 2, 6, Qt::AlignRight);

    // grid->addWidget(new QLabel("Size:"), 2, 0, 1, 1, Qt::AlignRight);
    // grid->addWidget(&_sizeSlider, 2, 1, 1, 7);

    vBox->addLayout(grid);
    vBox->addWidget(&_canvas);
    setLayout(vBox);

    connect(&_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    onInit();
}

FancySlider::FancySlider(QWidget *parent) : QSlider(parent)
{
}

FancySlider::FancySlider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent)
{
}

void FancySlider::sliderChange(QAbstractSlider::SliderChange change)
{
    QSlider::sliderChange(change);

    if (change == QAbstractSlider::SliderValueChange)
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        QPoint bottomRightCorner = sr.bottomLeft();

        QToolTip::showText(mapToGlobal(QPoint(bottomRightCorner.x(), bottomRightCorner.y())), QString::number(value()),
                           this);
    }
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