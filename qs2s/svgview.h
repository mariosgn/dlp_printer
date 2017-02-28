#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QObject>

#include <QGraphicsView>
#include <QTimer>

class QGraphicsSvgItem;
class QSvgRenderer;
class QWheelEvent;
class QPaintEvent;

class SvgView : public QGraphicsView
{
    Q_OBJECT

public:
    enum RendererType { Native, OpenGL, Image };

    explicit SvgView ( QWidget* parent = nullptr );

    bool openFile ( const QString& fileName );
    void setRenderer ( RendererType type = Native );
    void drawBackground ( QPainter* p, const QRectF& rect ) override;

    void startAnimation ( quint64 startDelayMs, quint64 intervalMs );

public slots:
    void setHighQualityAntialiasing ( bool highQualityAntialiasing );


private:
    bool readIds ( const QString& filename );

private slots:
    void showLayer ( );

private:
    QSvgRenderer* mp_Renderer = nullptr ;
    QGraphicsSvgItem* mp_SvgSingleLayer = nullptr;
    QList<QString> ml_SvgIds;
    RendererType ms_Renderer;
    QImage ms_Image;

    quint64 mi_LayerCurrentIndex;
    quint64 mi_Interval;
    QTimer ms_IntervalTimer;
};

#endif // SVGVIEW_H
