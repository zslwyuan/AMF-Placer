#include "qblcanvas.h"

QBLCanvas::QBLCanvas()
  : _rendererType(RendererBlend2D),
    _dirty(true),
    _fps(0),
    _frameCount(0) {
  _elapsedTimer.start();
  setMouseTracking(true);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QBLCanvas::~QBLCanvas() {}

void QBLCanvas::resizeEvent(QResizeEvent* event) {
  _resizeCanvas();
}

void QBLCanvas::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  if (_dirty)
    _renderCanvas();
  painter.drawImage(QPoint(0, 0), qtImage);
}

void QBLCanvas::mousePressEvent(QMouseEvent* event) {
  if (onMouseEvent)
    onMouseEvent(event);
}

void QBLCanvas::mouseReleaseEvent(QMouseEvent* event) {
  if (onMouseEvent)
    onMouseEvent(event);
}

void QBLCanvas::mouseMoveEvent(QMouseEvent* event) {
  if (onMouseEvent)
    onMouseEvent(event);
}

void QBLCanvas::setRendererType(uint32_t rendererType) {
  _rendererType = rendererType;
  updateCanvas();
}

void QBLCanvas::updateCanvas(bool force) {
  if (force)
    _renderCanvas();
  else
    _dirty = true;
  repaint(0, 0, width(), height());
}

void QBLCanvas::_resizeCanvas() {
  int w = width();
  int h = height();

  if (qtImage.width() == w && qtImage.height() == h)
    return;

  qtImage = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
  blImage.createFromData(qtImage.width(), qtImage.height(), BL_FORMAT_PRGB32, qtImage.bits(), qtImage.bytesPerLine());

  updateCanvas(false);
}

void QBLCanvas::_renderCanvas() {
  if (_rendererType == RendererQt) {
    if (onRenderQt) {
      QPainter ctx(&qtImage);
      onRenderQt(ctx);
    }
  }
  else {
    if (onRenderB2D) {
      // In Blend2D case the non-zero _rendererType specifies the number of threads.
      BLContextCreateInfo createInfo {};
      createInfo.threadCount = _rendererType;

      BLContext ctx(blImage, createInfo);
      onRenderB2D(ctx);
    }
  }

  _dirty = false;
  _afterRender();
}

void QBLCanvas::_afterRender() {
  uint64_t t = _elapsedTimer.elapsed();

  _frameCount++;
  if (t >= 1000) {
    _fps = _frameCount / double(t) * 1000.0;
    _frameCount = 0;
    _elapsedTimer.start();
  }
}

void QBLCanvas::initRendererSelectBox(QComboBox* dst) {
  static const uint32_t rendererTypes[] = {
    RendererQt,
    RendererBlend2D,
    RendererBlend2D_1t,
    RendererBlend2D_2t,
    RendererBlend2D_4t,
    RendererBlend2D_8t,
    RendererBlend2D_12t,
    RendererBlend2D_16t
  };

  for (const auto& rendererType : rendererTypes) {
    QString s = rendererTypeToString(rendererType);
    dst->addItem(s, QVariant(int(rendererType)));
  }

  dst->setCurrentIndex(1);
}

QString QBLCanvas::rendererTypeToString(uint32_t rendererType) {
  char buffer[32];
  switch (rendererType) {
    case RendererQt:
      return QLatin1String("Qt");

    default:
      if (rendererType > 32)
        return QString();

      if (rendererType == 0)
        return QLatin1String("Blend2D");

      snprintf(buffer, sizeof(buffer), "Blend2D %uT", rendererType);
      return QLatin1String(buffer);
  }
}

#include "moc_qblcanvas.cpp"
