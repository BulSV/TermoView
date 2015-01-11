#ifdef DEBUG
#include <QDebug>
#endif

#include <QApplication>
#include <QProxyStyle>
#include "Dialog.h"
#include <QStyleFactory>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);        

    app.setStyle(QStyleFactory::create("motif"));
#if defined (Q_OS_UNIX)
    app.setWindowIcon(QIcon(":/TermoViewIcon.png"));
#endif
    Dialog fDialog;
    fDialog.setWindowTitle(QString::fromUtf8("Termo View"));
    fDialog.show();

    return app.exec();
}

