#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>

int main ( int argc, char* argv[] )
{
    QApplication a ( argc, argv );

    QCoreApplication::setApplicationName ( "Svg to screeen" );
    QCoreApplication::setApplicationVersion ( "1.0" );

    QCommandLineParser parser;
    parser.setApplicationDescription ( "Test helper" );
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    // Fullscreen file
    QCommandLineOption fulscreenOption ( QStringList() << "f" << "no-fullscreen",
                                         QCoreApplication::translate ( "main", "Do not go fullscreen." ) );
    parser.addOption ( fulscreenOption );


    // SVG file
    QCommandLineOption svgOption ( QStringList() << "s" << "svg-file",
                                   QCoreApplication::translate ( "main", "The file to display." ),
                                   QCoreApplication::translate ( "main", "svgfile" ) );
    parser.addOption ( svgOption );


    parser.process ( a );

    const QStringList args = parser.positionalArguments();

    QString svgFile = parser.value ( svgOption );
    bool noFullscreen = parser.isSet ( fulscreenOption );


    MainWindow w ( svgFile );

    if ( noFullscreen)
        w.show();
    else
        w.showFullScreen();

    w.startAnimation( 200, 50 );

    return a.exec();
}
