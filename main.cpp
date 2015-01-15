#ifdef DEBUG
#include <QDebug>
#endif

#include <QApplication>
#include <QProxyStyle>
#include "Dialog.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

#if defined (Q_OS_UNIX)
    app.setWindowIcon(QIcon(":/Termo.png"));
#endif
    Dialog fDialog;
    fDialog.setWindowTitle(QString::fromUtf8("Termo View"));
    fDialog.show();

    return app.exec();
}

