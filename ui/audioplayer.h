#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QString>

class QMediaPlayer;
class QAudioOutput;

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject *parent = nullptr);

    // 在 resources/audio 下查找 <key>.mp3 / .wav / .ogg / .m4a / .flac，返回完整路径
    static QString findAudio(const QString &key);

    // 播放背景音乐（循环）；找不到文件返回 false，界面据此给出提示
    bool playBackground(const QString &key, int volumePercent = 55);
    void stopBackground();
    bool isBackgroundPlaying() const;
    qint64 backgroundPositionMs() const; 

    // 播放一次性音效（题目音频）
    void playEffect(const QString &key, int volumePercent = 80);

    // 题目音频场景
    void suspendBackground(); // 暂停配乐并记住状态
    void resumeBackground();  // 从暂停的位置接着播放
    void stopEffect();        // 立即停止当前正在播放的题目音频

    // 音频未配置时给出放置说明
    static QString audioFolderHint();

private:
    QMediaPlayer *m_player = nullptr; // 背景配乐
    QAudioOutput *m_output = nullptr;
    QMediaPlayer *m_effect = nullptr; // 题目音频
    QString m_currentKey;
    int m_volume = 55;
    bool m_playing = false;
    bool m_suspended = false;
};

#endif // AUDIOPLAYER_H
