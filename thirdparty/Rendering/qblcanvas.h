#ifndef QBLCANVAS_H
#define QBLCANVAS_H

#include <blend2d.h>
#include <cmath>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <QtGui>
#include <QtWidgets>
#include <functional>

class QBLCanvas : public QWidget
{
    Q_OBJECT

  public:
    QImage qtImage;
    BLImage blImage;

    enum RendererType : uint32_t
    {
        RendererBlend2D = 0
    };

    uint32_t _rendererType;
    bool _dirty;
    double _fps;
    uint32_t _frameCount;
    QElapsedTimer _elapsedTimer;

    std::function<void(BLContext &ctx)> onRenderB2D;
    std::function<void(QMouseEvent *)> onMouseEvent;
    std::function<void(QKeyEvent *)> onKeyPressEvent;
    std::function<void(QKeyEvent *)> onKeyReleaseEvent;

    QBLCanvas();
    ~QBLCanvas();

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void setRendererType(uint32_t rendererType);
    void updateCanvas(bool force = false);
    void _resizeCanvas();
    void _renderCanvas();
    void _afterRender();

    inline uint32_t rendererType() const
    {
        return _rendererType;
    }
    inline double fps() const
    {
        return _fps;
    }

    static void initRendererSelectBox(QComboBox *dst);
    static QString rendererTypeToString(uint32_t rendererType);
};

#endif // QBLCANVAS_H
