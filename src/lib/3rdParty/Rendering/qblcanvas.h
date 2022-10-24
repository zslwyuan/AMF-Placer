#ifndef QBLCANVAS_H
#define QBLCANVAS_H

#include <blend2d.h>
#include <cmath>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include <QtGui>
#include <QtWidgets>
#include <functional>

class QBLCanvas : public QWidget {
  Q_OBJECT

public:
  QImage qtImage;
  BLImage blImage;

  enum RendererType : uint32_t {
    RendererBlend2D = 0,
    RendererBlend2D_1t = 1,
    RendererBlend2D_2t = 2,
    RendererBlend2D_4t = 4,
    RendererBlend2D_8t = 8,
    RendererBlend2D_12t = 12,
    RendererBlend2D_16t = 16,

    RendererQt = 0xFF
  };

  uint32_t _rendererType;
  bool _dirty;
  double _fps;
  uint32_t _frameCount;
  QElapsedTimer _elapsedTimer;

  std::function<void(BLContext& ctx)> onRenderB2D;
  std::function<void(QPainter& ctx)> onRenderQt;
  std::function<void(QMouseEvent*)> onMouseEvent;

  QBLCanvas();
  ~QBLCanvas();

  void resizeEvent(QResizeEvent* event) override;
  void paintEvent(QPaintEvent *event) override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

  void setRendererType(uint32_t rendererType);
  void updateCanvas(bool force = false);
  void _resizeCanvas();
  void _renderCanvas();
  void _afterRender();

  inline uint32_t rendererType() const { return _rendererType; }
  inline double fps() const { return _fps; }

  static void initRendererSelectBox(QComboBox* dst);
  static QString rendererTypeToString(uint32_t rendererType);
};

#endif // QBLCANVAS_H
