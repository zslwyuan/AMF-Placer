#ifndef _blqtAMF_
#define _blqtAMF_

#include "bl-qt-static.h"
#include "paintDB.h"
#include "qblcanvas.h"
#include <QMouseEvent>
#include <stdlib.h>
#include <vector>
#define WIN_W 400.0
#define WIN_H 970.0

static QPainter::CompositionMode Blend2DCompOpToQtCompositionMode(BLCompOp compOp);

class MainWindow : public QWidget
{
    Q_OBJECT

  public:
    QTimer _timer;
    QSlider _sizeSlider;
    QSlider _countSlider;
    QComboBox _rendererSelect;
    QComboBox _compOpSelect;
    QComboBox _shapeTypeSelect;
    QCheckBox _limitFpsCheck;
    QBLCanvas _canvas;
    PaintDataBase *paintData = nullptr;

    BLRandom _random;
    std::vector<BLPoint> _coords;
    std::vector<int> types;
    // LUT,FF,MUX,CARRY,DSP,BRAM,LUTRAM
    double _type2W[10] = {1, 1, 1, 1, 4, 4, 1, 3};
    double _type2H[10] = {1, 1, 1, 2, 5, 7.5, 2, 900};
    std::vector<BLRgba32> _type2C;
    std::vector<int> elementTypes;
    std::vector<int> timingPath;
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
    }

    void onInit()
    {
        setCount(_countSlider.sliderPosition());
        _limitFpsCheck.setChecked(true);
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
    Q_SLOT void onSizeChanged(int value)
    {
        _rectSize = value;
    }
    Q_SLOT void onCountChanged(int value)
    {
        setCount(size_t(value));
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
        if (paintData->readElementInfo(Xs, Ys, elementTypes, timingPath))
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
        W = _type2W[type] / 2 * w / curW;
        H = _type2H[type] / 2 * h / curH;
        if (fineGrainedShow)
        {
            W = W / shrinkRatio;
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
            double XInWin = x / WIN_W * w;
            double YInWin = y / WIN_H * h;
            x = (XInWin - winLeft) / curW * w;
            y = (YInWin - winBottom) / curH * h;
        }
        else
        {
            if (fineGrainedShow)
            {
                x += (i % 8 + 1) * 0.15 - 0.75;
                y += (i / 8 % 8 + 1) * 0.15 - 0.75;
            }
            double XInWin = x / WIN_W * w;
            double YInWin = y / WIN_H * h;
            x = (XInWin - winLeft) / curW * w;
            y = (YInWin - winBottom) / curH * h;
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
        for (int t = 0; t < 7; t++)
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
                x = x - halfW;
                y = y - halfH;
                ctx.setFillStyle(curTypeC);
                ctx.fillRect(x, y, halfW * 2, halfH * 2);
            }

        if (timingPath.size())
        {
            for (int i = 1; i < timingPath.size(); i++)
            {
                BLPath path;
                double x = _coords[timingPath[i - 1]].x;
                double y = _coords[timingPath[i - 1]].y;
                remapXY(x, y, i);
                path.moveTo(x, y);
                x = _coords[timingPath[i]].x;
                y = _coords[timingPath[i]].y;
                remapXY(x, y, i);
                path.lineTo(x, y);
                ctx.setStrokeStyle(BLRgba32(0xFF00FF00));
                ctx.setCompOp(BL_COMP_OP_SRC_OVER);
                ctx.setStrokeWidth(2);
                ctx.strokePath(path);
                path.clear();
                path.close();
            }
        }

        ctx.setFillStyle(BLRgba32(0x80000000));
        ctx.fillRect(10, h - (900) / WIN_H * h - 20, 100, h - (900) / WIN_H * h - 20 + (30 * 8) / WIN_H * h);

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
        ctx.fillUtf8Text(BLPoint(10, h - (900) / WIN_H * h), font, "LUT");
        ctx.setFillStyle(BLRgba32(0xFFFF0000));
        ctx.fillUtf8Text(BLPoint(10, h - (900 - 30 * 1) / WIN_H * h), font, "FF");
        ctx.setFillStyle(BLRgba32(0xFF00FF00));
        ctx.fillUtf8Text(BLPoint(10, h - (900 - 30 * 2) / WIN_H * h), font, "MUX");
        ctx.setFillStyle(BLRgba32(0xFFFFFF00));
        ctx.fillUtf8Text(BLPoint(10, h - (900 - 30 * 3) / WIN_H * h), font, "CARRY");
        ctx.setFillStyle(BLRgba32(0xFFFF00FF));
        ctx.fillUtf8Text(BLPoint(10, h - (900 - 30 * 4) / WIN_H * h), font, "DSP");
        ctx.setFillStyle(BLRgba32(0xFFFFFFFF));
        ctx.fillUtf8Text(BLPoint(10, h - (900 - 30 * 5) / WIN_H * h), font, "BRAM");
        ctx.setFillStyle(BLRgba32(0xFF2E2EFF));
        ctx.fillUtf8Text(BLPoint(10, h - (900 - 30 * 6) / WIN_H * h), font, "LUTRAM");
        ctx.setFillStyle(BLRgba32(0xA4ECFF83));
        ctx.fillUtf8Text(BLPoint(10, h - (900 - 30 * 7) / WIN_H * h), font, "Blockage");
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
        double w = _canvas.blImage.width();
        double h = _canvas.blImage.height();

        double realCX = x / w * curW + winLeft;
        double realCY = y / h * curH + winBottom;

        double changeStep = 0.9;
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
            shrinkRatio = std::pow(w / curW, 0.5);
        }
        else
        {
            fineGrainedShow = false;
        }
    }

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