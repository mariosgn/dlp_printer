#include "svgview.h"

#include <QSvgRenderer>

#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QPaintEvent>
#include <QDebug>
#include <qmath.h>

#ifndef QT_NO_OPENGL
#include <QFileInfo>
#include <QGLWidget>
#endif

SvgView::SvgView ( QWidget* parent ) :
    QGraphicsView ( parent ),
    ms_Renderer ( Native )
{
    setScene ( new QGraphicsScene ( this ) );
    setViewportUpdateMode ( FullViewportUpdate );

    QPixmap tilePixmap ( 64, 64 );
    tilePixmap.fill ( Qt::black );
    setBackgroundBrush ( tilePixmap );

    connect ( &ms_IntervalTimer, &QTimer::timeout, this, &SvgView::showLayer );
    ms_IntervalTimer.setSingleShot ( true );
    ms_LastOutput.start();
}

void SvgView::drawBackground ( QPainter* p, const QRectF& )
{
    p->save();
    p->resetTransform();
    p->drawTiledPixmap ( viewport()->rect(), backgroundBrush().texture() );
    p->restore();
}



bool SvgView::openFile ( const QString& fileName )
{

    if ( ! readIds ( fileName ) )
    {
        return false;
    }

    QGraphicsScene* s = scene();

    mp_Renderer = new QSvgRenderer ( fileName );
    mp_SvgSingleLayer = new QGraphicsSvgItem () ;
    mp_SvgSingleLayer->setSharedRenderer ( mp_Renderer );

    if ( !mp_Renderer->isValid() )
        return false;

    s->clear();
    resetTransform();

    mp_SvgSingleLayer->setFlags ( QGraphicsItem::ItemClipsToShape );
    mp_SvgSingleLayer->setCacheMode ( QGraphicsItem::NoCache );
    mp_SvgSingleLayer->setZValue ( 0 );
    mp_SvgSingleLayer->hide();

    s->addItem ( mp_SvgSingleLayer );
    return true;
}

void SvgView::setRenderer ( RendererType type )
{
    ms_Renderer = type;

    if ( ms_Renderer == OpenGL )
    {
#ifndef QT_NO_OPENGL
        setViewport ( new QGLWidget ( QGLFormat ( QGL::SampleBuffers ) ) );
#endif
    }
    else
    {
        setViewport ( new QWidget );
    }
}

void SvgView::setHighQualityAntialiasing ( bool highQualityAntialiasing )
{
#ifndef QT_NO_OPENGL
    setRenderHint ( QPainter::HighQualityAntialiasing, highQualityAntialiasing );
#else
    Q_UNUSED ( highQualityAntialiasing );
#endif
}




bool SvgView::readIds ( const QString& fileName )
{
    QFileInfo fi ( fileName );
    QFile file ( fileName );
    if ( !file.exists() || !file.open ( QIODevice::ReadOnly ) )
    {
        qDebug() <<  "Cannot open xml image file: " <<  fi.fileName() ;
        return false;
    }

    QXmlStreamReader xml;
    xml.setDevice ( &file );

    QString tag;
    while ( !xml.atEnd() )
    {
        if ( xml.hasError() )
        {
            return false;
        }

        QXmlStreamReader::TokenType t = xml.readNext();
        if ( t == QXmlStreamReader::Invalid )
        {
            return false;
        }
        else if ( t == QXmlStreamReader::StartElement )
        {
            tag = QString ( xml.name().constData(), xml.name().size() );
            if ( tag == "g" )
            {
                QXmlStreamAttributes attr = xml.attributes();
                if ( !attr.hasAttribute ( "id" ) )
                    return false;
                ml_SvgIds.append (  attr.value ( "id" ).toString() ) ;
            }
        }
    }

//    std::sort ( ml_SvgIds.begin(), ml_SvgIds.end() );

    return ml_SvgIds.size() > 0;
}

void SvgView::startAnimation ( quint64 startDelayMs, quint64 intervalMs )
{
    mi_LayerCurrentIndex = 0;
    mi_Interval = intervalMs;
    ms_IntervalTimer.start ( startDelayMs );
}

void SvgView::showLayer()
{
    QString currId = ml_SvgIds.at ( mi_LayerCurrentIndex++ );

    if ( ms_LastOutput.elapsed() > 1000 * 1 )
    {
        int perc = (mi_LayerCurrentIndex*100 ) / ml_SvgIds.size();
        QString outstr = QString( "{\"mtype\": \"status\", \"perc\": \"%1\"}\n").arg( perc );
        QTextStream out(stdout, QIODevice::WriteOnly);
        out << outstr;
        out.flush();

        ms_LastOutput.restart();
    }

    mp_SvgSingleLayer->setElementId ( currId );
    mp_SvgSingleLayer->show();
    ms_IntervalTimer.start ( mi_Interval );
}
