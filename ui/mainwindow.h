#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../model/game.h"

#include <QColor>
#include <QMainWindow>
#include <QPixmap>
#include <QString>

class AudioPlayer;
class BoardWidget;
class QTimer;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 主窗口：界面背景、棋盘、玩家面板、回合流程、弹窗与配乐
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, int autoPlayers = 0, bool selfCheck = false,
                        bool noMusic = false);
    ~MainWindow() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void startNewGame();
    void onRollDice();
    void onMoveStep();
    void onToggleMusic();
    void onShowRules();
    void onShowAbout();

private:
    void setupUi();
    void applyScreenAdaptiveSize(); 
    void handleLanding();
    void handleQuiz(int landIndex);
    void finishTurn();
    void refreshBoard();
    void refreshPlayers();
    void refreshStatus();
    void appendLog(const QString &text, const QString &color = QString());
    void setBusy(bool busy);
    void announceGameOver();
    void startBackgroundMusic();
    void runLayoutSelfCheck(); 
    QString nameOf(int playerIndex) const;
    QColor playerColor(int playerIndex) const;

    Ui::MainWindow *ui;
    BoardWidget *m_board = nullptr;
    QTimer *m_moveTimer = nullptr;
    AudioPlayer *m_audio = nullptr;
    Game m_game;
    QPixmap m_background;
    bool m_busy = false;
    bool m_overAnnounced = false;
    bool m_musicEnabled = true;
    bool m_musicHinted = false;
    int m_autoPlayers = 0;
    bool m_selfCheck = false;
};

#endif // MAINWINDOW_H
