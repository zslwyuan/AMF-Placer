#ifndef _blqtAMF_
#define _blqtAMF_

#include "bl-qt-static.h"
#include "paintDB.h"
#include "qblcanvas.h"
#include <QMouseEvent>
#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <QKeyEvent>

#define WIN_W 400.0
#define WIN_H 970.0

static QPainter::CompositionMode Blend2DCompOpToQtCompositionMode(BLCompOp compOp);

class FancySlider : public QSlider
{
    Q_OBJECT
  public:
    explicit FancySlider(QWidget *parent = 0);
    explicit FancySlider(Qt::Orientation orientation, QWidget *parent = 0);

  protected:
    virtual void sliderChange(SliderChange change);
};

class MainWindow : public QWidget
{
    Q_OBJECT

  public:
    QTimer _timer;
    QSlider _sizeSlider;
    FancySlider _countSlider;
    QComboBox _rendererSelect;
    QComboBox _compOpSelect;
    QComboBox _shapeTypeSelect;

    QCheckBox _LUTCheck;
    QCheckBox _FFCheck;
    QCheckBox _MUXCheck;
    QCheckBox _CARRYCheck;
    QCheckBox _DSPCheck;
    QCheckBox _BRAMCheck;
    QCheckBox _OtherCheck;

    QBLCanvas _canvas;
    PaintDataBase *paintData = nullptr;

    BLRandom _random;
    std::vector<BLPoint> _coords;
    std::vector<int> types;
    // LUT,FF,MUX,CARRY,DSP,BRAM,LUTRAM
    double _type2W[10] = {1, 1, 1, 1, 4, 4, 1, 3};
    double _type2H[10] = {1, 1, 1, 2, 5, 7.5, 2, 960};
    std::vector<BLRgba32> _type2C;
    std::vector<int> elementTypes;
    std::vector<std::vector<int>> timingPaths;
    std::vector<std::string> cellNames;
    std::vector<BLPoint> _steps;
    std::vector<BLRgba32> _colors;
    BLCompOp _compOp;
    uint32_t _shapeType;
    double _rectSize;
    double winLeft = 0;
    double winRight = WIN_W;
    double winTop = WIN_H;
    double winBottom = 0;
    double curW = WIN_W;
    double curH = WIN_H;
    bool lastIsButtonPressed = false;
    bool fineGrainedShow = false;
    double shrinkRatio = 1.0;
    bool toFine = false;
    bool toCoarse = false;
    bool enableCellNameWatch = false;
    int pathNum = 1;

    double mouseX = -1;
    double mouseY = -1;

    bool showTypes[10] = {true, true, true, true, true, true, true, true, true, true};

    enum ShapeType
    {
        kShapeRect,
    };

    MainWindow();

    void showEvent(QShowEvent *event) override
    {
        _timer.start();
    }
    void hideEvent(QHideEvent *event) override
    {
        _timer.stop();
    }
    void keyPressEvent(QKeyEvent *event) override
    {
        switch (event->key())
        {
        case Qt::Key_W:
            enableCellNameWatch = true;
            break;
        }
    }
    void keyReleaseEvent(QKeyEvent *event) override
    {
        switch (event->key())
        {
        case Qt::Key_W:
            enableCellNameWatch = false;
            break;
        }
    }
    void onInit()
    {
        setCount(_countSlider.sliderPosition());
        _LUTCheck.setChecked(true);
        _FFCheck.setChecked(true);
        _MUXCheck.setChecked(true);
        _CARRYCheck.setChecked(true);
        _DSPCheck.setChecked(true);
        _BRAMCheck.setChecked(true);
        _OtherCheck.setChecked(true);
        _updateTitle();
    }

    double randomSign() noexcept
    {
        return _random.nextDouble() < 0.5 ? 1.0 : -1.0;
    }
    BLRgba32 randomColor() noexcept
    {
        return BLRgba32(_random.nextUInt32());
    }

    Q_SLOT void onRendererChanged(int index)
    {
        _canvas.setRendererType(_rendererSelect.itemData(index).toInt());
    }
    Q_SLOT void onCompOpChanged(int index)
    {
        _compOp = (BLCompOp)_compOpSelect.itemData(index).toInt();
    };
    Q_SLOT void onShapeTypeChanged(int index)
    {
        _shapeType = _shapeTypeSelect.itemData(index).toInt();
    };
    Q_SLOT void onLimitFpsChanged(int value)
    {
        _timer.setInterval(value ? 1000 / 120 : 0);
    }

    Q_SLOT void onLUTChanged(int value)
    {
        showTypes[0] = value;
    }

    Q_SLOT void onFFChanged(int value)
    {
        showTypes[1] = value;
    }

    Q_SLOT void onMUXChanged(int value)
    {
        showTypes[2] = value;
    }

    Q_SLOT void onCARRYChanged(int value)
    {
        showTypes[3] = value;
    }

    Q_SLOT void onDSPChanged(int value)
    {
        showTypes[4] = value;
    }

    Q_SLOT void onBRAMChanged(int value)
    {
        showTypes[5] = value;
    }

    Q_SLOT void onOtherChanged(int value)
    {
        showTypes[6] = value;
    }

    Q_SLOT void onSizeChanged(int value)
    {
        _rectSize = value;
    }
    Q_SLOT void onCountChanged(int value)
    {
        pathNum = value;
        paintData->setPaintDemand(value);
    }

    Q_SLOT void onTimer()
    {
        double w = _canvas.blImage.width();
        double h = _canvas.blImage.height();

        std::vector<float> Xs, Ys;

        // double IOX6[6] = {7.5, 15.5, 33.5, 51.5, 69.5, 76.5};
        // _coords.resize(6);
        // for (size_t i = 0; i < 6; i++)
        // {
        //     BLPoint &vertex = _coords[i];
        //     vertex.x = (Xs[i] * 4 + 5);
        //     vertex.y = (WIN_H - (Ys[i] * 2 + 5));
        // }

        assert(paintData);
        if (paintData->readElementInfo(Xs, Ys, elementTypes, timingPaths, cellNames))
        {
            _coords.resize(Xs.size() + 6);
            size_t size = Xs.size();
            for (size_t i = 0; i < size; i++)
            {
                BLPoint &vertex = _coords[i];
                vertex.x = (Xs[i] * 4 + 5);
                vertex.y = (WIN_H - (Ys[i] * 2) - 5);
            }

            double IOX6[6] = {7, 15.5, 33.5, 52.5, 71.5, 78.25};
            for (size_t i = 0; i < 6; i++)
            {
                BLPoint &vertex = _coords[size + i];
                vertex.x = (IOX6[i] * 4 + 5);
                vertex.y = (WIN_H / 2);
                elementTypes.push_back(7);
            }
        }

        _canvas.updateCanvas(true);
        _updateTitle();
    }

    void getWHC(int type, double &W, double &H, BLRgba32 &c, int index)
    {
        // // LUT,FF,MUX,CARRY,DSP,BRAM,LUTRAM
        // double _type2W[10] = {1, 1, 1, 1, 2, 2, 1};
        // double _type2H[10] = {1, 1, 1, 2, 5, 7.5, 2};
        // std::vector<BLRgba32> _type2C;

        double w = _canvas.blImage.width();
        double h = _canvas.blImage.height();
        assert(type >= 0 && type <= 7);
        W = _type2W[type] / 2 * WIN_W / curW;
        H = _type2H[type] / 2 * WIN_H / curH;
        if (fineGrainedShow)
        {
            W = W / shrinkRatio;
            if (type != 7)
                H = H / shrinkRatio;
        }
        if (type < 4)
            c = BLRgba32(_type2C[type].value | (index & 0x050505));
        else
            c = _type2C[type];
    }

    void remapXY(double &x, double &y, int i = -1)
    {
        double w = _canvas.blImage.width();
        double h = _canvas.blImage.height();

        if (i < 0)
        {
            double XInWin = x / WIN_W * WIN_W;
            double YInWin = y / WIN_H * WIN_H;
            x = (XInWin - winLeft) / curW * WIN_W;
            y = (YInWin - winBottom) / curH * WIN_H;
        }
        else
        {
            if (fineGrainedShow)
            {
                x += (i % 8 + 1) * 0.15 - 0.75;
                y += (i / 8 % 8 + 1) * 0.15 - 0.75;
            }
            double XInWin = x / WIN_W * WIN_W;
            double YInWin = y / WIN_H * WIN_H;
            x = (XInWin - winLeft) / curW * WIN_W;
            y = (YInWin - winBottom) / curH * WIN_H;
        }
    }

    void transformDeviceXY(double &X, double &Y)
    {
        X = (X * 4 + 5);
        Y = (WIN_H - (Y * 2) - 5);
    }

    void onRenderB2D(BLContext &ctx) noexcept
    {
        double w = _canvas.blImage.width();
        double h = _canvas.blImage.height();
        ctx.setFillStyle(BLRgba32(0xFF000000));
        ctx.fillAll();

        double boundaryXs[5] = {0, 90, 90, 0, 0};
        double boundaryYs[5] = {0, 0, 480, 480, 0};
        BLPath path;
        double x = boundaryXs[0], y = boundaryYs[0];
        transformDeviceXY(x, y);
        remapXY(x, y);
        path.moveTo(x, y);
        for (int i = 1; i < 5; i++)
        {
            double x = boundaryXs[i], y = boundaryYs[i];
            transformDeviceXY(x, y);
            remapXY(x, y);
            path.lineTo(x, y);
        }

        ctx.setStrokeStyle(BLRgba32(0xFFFFFFFF));
        ctx.setCompOp(BL_COMP_OP_SRC_OVER);
        ctx.setStrokeWidth(2);
        ctx.setStrokeStartCap(BL_STROKE_CAP_ROUND);
        ctx.setStrokeEndCap(BL_STROKE_CAP_BUTT);
        ctx.strokePath(path);

        size_t i;
        size_t size = _coords.size();

        ctx.setCompOp(_compOp);
        for (i = 0; i < size; i++)
        {
            if (7 != elementTypes[i])
                continue;
            double halfW, halfH;
            BLRgba32 curTypeC;
            getWHC(elementTypes[i], halfW, halfH, curTypeC, i);
            double x = _coords[i].x;
            double y = _coords[i].y;
            remapXY(x, y);
            x = x - halfW;
            y = y - halfH;

            ctx.setFillStyle(curTypeC);
            ctx.fillRect(x, y, halfW * 2, halfH * 2);
        }

        int findCloseCellId = -1;
        double closeDis = 1000000000;
        double closeCX = -1;
        double closeCY = -1;
        for (int t = 0; t < 7; t++)
        {
            if (!showTypes[t])
                continue;
            for (i = 0; i < size; i++)
            {
                if (t != elementTypes[i])
                    continue;
                double halfW, halfH;
                BLRgba32 curTypeC;
                getWHC(elementTypes[i], halfW, halfH, curTypeC, i);
                double x = _coords[i].x;
                double y = _coords[i].y;
                remapXY(x, y, i);
                if (enableCellNameWatch && mouseX > 0 && mouseY > 0 && mouseX < w && mouseY < h)
                {
                    double dx = mouseX - x;
                    double dy = mouseY - y;
                    double dis = sqrt(dx * dx + dy * dy);
                    if (dis < closeDis)
                    {
                        closeCX = x;
                        closeCY = y;
                        closeDis = dis;
                        findCloseCellId = i;
                    }
                }
                x = x - halfW;
                y = y - halfH;
                ctx.setFillStyle(curTypeC);
                ctx.fillRect(x, y, halfW * 2, halfH * 2);
            }
        }

        if (timingPaths.size())
        {
            int pathCnt = 0;
            for (auto timingPath : timingPaths)
            {
                pathCnt++;
                ctx.setStrokeStyle(
                    BLRgba32(0xFF000000 + (std::hash<std::string>{}(std::to_string(pathCnt)) & 0x00FFFFFF)));
                for (int i = 1; i < timingPath.size(); i++)
                {
                    BLPath path;
                    double x = _coords[timingPath[i - 1]].x;
                    double y = _coords[timingPath[i - 1]].y;
                    remapXY(x, y, timingPath[i - 1]);
                    path.moveTo(x, y);
                    x = _coords[timingPath[i]].x;
                    y = _coords[timingPath[i]].y;
                    remapXY(x, y, timingPath[i]);
                    path.lineTo(x, y);
                    ctx.setCompOp(BL_COMP_OP_SRC_OVER);
                    ctx.setStrokeWidth(2);
                    ctx.strokePath(path);
                    path.clear();
                    path.close();
                }
                if (pathCnt >= pathNum)
                    break;
            }
        }

        ctx.setFillStyle(BLRgba32(0x80000000));
        ctx.fillRect(10, 5, 100, 20 + (30 * 8));

        BLFontFace face;
        BLResult err = face.createFromFile("NotoSans-Regular.ttf");
        if (err)
        {
            printf("Failed to load a font-face (err=%u)\n", err);
            assert(false);
        }
        BLFont font;
        font.createFromFace(face, 20.0f);
        ctx.setFillStyle(BLRgba32(0xFF00FFFF));
        ctx.fillUtf8Text(BLPoint(10, (25)), font, "LUT");
        ctx.setFillStyle(BLRgba32(0xFFFF0000));
        ctx.fillUtf8Text(BLPoint(10, (25 + 30 * 1)), font, "FF");
        ctx.setFillStyle(BLRgba32(0xFF00FF00));
        ctx.fillUtf8Text(BLPoint(10, (25 + 30 * 2)), font, "MUX");
        ctx.setFillStyle(BLRgba32(0xFFFFFF00));
        ctx.fillUtf8Text(BLPoint(10, (25 + 30 * 3)), font, "CARRY");
        ctx.setFillStyle(BLRgba32(0xFFFF00FF));
        ctx.fillUtf8Text(BLPoint(10, (25 + 30 * 4)), font, "DSP");
        ctx.setFillStyle(BLRgba32(0xFFFFFFFF));
        ctx.fillUtf8Text(BLPoint(10, (25 + 30 * 5)), font, "BRAM");
        ctx.setFillStyle(BLRgba32(0xFF2E2EFF));
        ctx.fillUtf8Text(BLPoint(10, (25 + 30 * 6)), font, "LUTRAM");
        ctx.setFillStyle(BLRgba32(0xA4ECFF83));
        ctx.fillUtf8Text(BLPoint(10, (25 + 30 * 7)), font, "Blockage");

        if (enableCellNameWatch && findCloseCellId >= 0)
        {

            BLFont font;
            font.createFromFace(face, 15.0f);

            ctx.setFillStyle(BLRgba32(0xFFFFFFFF));
            auto lines = wraptext(cellNames[findCloseCellId], 30);
            for (int i = 0; i < lines.size(); i++)
                ctx.fillUtf8Text(BLPoint(closeCX + 10, closeCY + i * 17), font, lines[i].c_str());
        }
    }

    std::vector<std::string> wraptext(std::string input, size_t width)
    {
        size_t curpos = 0;
        size_t nextpos = 0;

        std::vector<std::string> lines;

        for (int i = 0; i < input.size(); i += width)
        {
            std::string substr = input.substr(i, width);
            lines.push_back(substr);
        }

        return lines;
    }

    void onRenderQt(QPainter &ctx) noexcept
    {
        assert(false && "Qt rendering is not ready yet");
        // ctx.fillRect(0, 0, _canvas.width(), _canvas.height(), QColor(0, 0, 0));

        // ctx.setRenderHint(QPainter::Antialiasing, true);
        // ctx.setCompositionMode(Blend2DCompOpToQtCompositionMode(_compOp));

        // size_t i;
        // size_t size = _coords.size();

        // switch (_shapeType)
        // {
        // case kShapeRect:
        //     for (int t = 0; t < 7; t++)
        //         for (i = 0; i < size; i++)
        //         {
        //             if (t != elementTypes[i])
        //                 continue;
        //             double halfW, halfH;
        //             BLRgba32 curTypeC;
        //             getWHC(elementTypes[i], halfW, halfH, curTypeC, i);
        //             double x = _coords[i].x - halfW;
        //             double y = _coords[i].y - halfH;

        //             ctx.fillRect(QRectF(_coords[i].x - halfW, _coords[i].y - halfH, halfW * 2, halfH * 2),
        //                          QColor(_colors[i].r(), _colors[i].g(), _colors[i].b(), _colors[i].a()));
        //         }
        //     break;
        // }
    }

    void onMouseEvent(QMouseEvent *event) noexcept
    {
        double x = event->x();
        double y = event->y();
        mouseX = x;
        mouseY = y;

        double w = _canvas.blImage.width();
        double h = _canvas.blImage.height();

        double realCX = x / WIN_W * curW + winLeft;
        double realCY = y / WIN_H * curH + winBottom;

        double changeStep = 0.75;
        if (!lastIsButtonPressed && (event->buttons() & Qt::LeftButton))
        {
            // std::cout << "========================================================\n";
            curW = curW * changeStep;
            curH = curH * changeStep;
            winLeft = realCX - changeStep * (realCX - winLeft);
            winBottom = realCY - changeStep * (realCY - winBottom);
            lastIsButtonPressed = true;
            // std::cout << " x=" << x << " y=" << y << " curW=" << curW << " curH=" << curH << " winLeft=" << winLeft
            //           << " winRight=" << winRight << "\n";
        }
        else if (!lastIsButtonPressed && (event->buttons() & Qt::RightButton))
        {
            // std::cout << "========================================================\n";
            curW = curW / changeStep;
            curH = curH / changeStep;
            winLeft = realCX - 1 / changeStep * (realCX - winLeft);
            winBottom = realCY - 1 / changeStep * (realCY - winBottom);
            lastIsButtonPressed = true;
            // std::cout << " x=" << x << " y=" << y << " curW=" << curW << " curH=" << curH << " winLeft=" << winLeft
            //           << " winRight=" << winRight << "\n";
        }
        else if (!lastIsButtonPressed && (event->buttons() & Qt::MiddleButton))
        {
            // std::cout << "========================================================\n";
            winLeft = 0;
            winRight = WIN_W;
            winTop = WIN_H;
            winBottom = 0;
            curW = WIN_W;
            curH = WIN_H;
            lastIsButtonPressed = true;
            // std::cout << " x=" << x << " y=" << y << " curW=" << curW << " curH=" << curH << " winLeft=" << winLeft
            //           << " winRight=" << winRight << "\n";
        }
        else
        {
            if (!event->buttons())
                lastIsButtonPressed = false;
        }
        toFine = false;
        if (w / curW > 20)
        {
            fineGrainedShow = true;
            shrinkRatio = std::pow(WIN_W / curW, 0.5);
        }
        else
        {
            fineGrainedShow = false;
        }
    }

    // void onKeyPressEvent(QKeyEvent *event) noexcept
    // {
    //     switch (event->key())
    //     {
    //     case Qt::Key_W:
    //         enableCellNameWatch = true;
    //         std::cout << "enableCellNameWatch\n";
    //         break;
    //     }
    // }

    // void onKeyReleaseEvent(QKeyEvent *event) noexcept
    // {
    //     switch (event->key())
    //     {
    //     case Qt::Key_W:
    //         enableCellNameWatch = false;
    //         break;
    //     }
    // }

    void setCount(size_t size)
    {
        // double w = _canvas.blImage.width();
        // double h = _canvas.blImage.height();
        // size_t i = _coords.size();

        // _coords.resize(size);
        // _steps.resize(size);
        // _colors.resize(size);

        // while (i < size)
        // {
        //     _coords[i].reset(_random.nextDouble() * w, _random.nextDouble() * h);
        //     _steps[i].reset((_random.nextDouble() * 0.5 + 0.05) * randomSign(),
        //                     (_random.nextDouble() * 0.5 + 0.05) * randomSign());
        //     _colors[i].reset(randomColor());
        //     i++;
        // }
    }

    void _updateTitle()
    {
        char buf[256];
        snprintf(buf, 256, " [Device:%dx%d] [#Cell=%zu] [%.1f FPS]", _canvas.width(), _canvas.height(), _coords.size(),
                 _canvas.fps());

        QString title = QString::fromUtf8(buf);
        if (title != windowTitle())
            setWindowTitle(title);
    }
};

// int main(int argc, char *argv[]) {
//   QApplication app(argc, argv);
//   MainWindow win;

//   win.setMinimumSize(QSize(400, 320));
//   win.resize(QSize(580, 520));
//   win.show();

//   return app.exec();
// }
#endif