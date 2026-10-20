#include "ui/mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("晋曲棋旅"));
    app.setApplicationDisplayName(QStringLiteral("晋曲棋旅 — 晋剧文化主题大富翁"));
    app.setOrganizationName(QStringLiteral("课程设计"));

    // 中文界面字体
    QFont font = app.font();
    font.setFamily(QStringLiteral("Microsoft YaHei"));
    font.setPointSize(10);
    app.setFont(font);

    int autoPlayers = 0;
    bool selfCheck = false;
    bool noMusic = false;
    const QStringList args = QCoreApplication::arguments();
    for (int i = 1; i < args.size(); ++i) {
        const QString &a = args.at(i);
        if (a == QStringLiteral("--players") && i + 1 < args.size())
            autoPlayers = args.at(++i).toInt();
        else if (a.startsWith(QStringLiteral("--players=")))
            autoPlayers = a.section(QLatin1Char('='), 1).toInt();
        else if (a == QStringLiteral("--selfcheck"))
            selfCheck = true;
        else if (a == QStringLiteral("--no-music"))
            noMusic = true;
    }

    MainWindow window(nullptr, autoPlayers, selfCheck, noMusic);
    window.show();
    return app.exec();
}
