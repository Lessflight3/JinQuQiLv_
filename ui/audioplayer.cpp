#include "audioplayer.h"

#include "artwork.h"

#include <QAudioOutput>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QStringList>
#include <QUrl>

namespace {

// 在 resources/audio/ 下依次尝试 <key>、<key>.mp3、<key>.wav ……，返回所有命中的完整路径
QStringList candidatePaths(const QString &key)
{
    const QStringList names = {key, key + ".mp3", key + ".wav",
                               key + ".ogg", key + ".m4a", key + ".flac"};
    QStringList paths;
    for (const QString &root : Artwork::resourceRoots()) {
        for (const QString &name : names) {
            const QString full = root + "audio/" + name;
            if (QFileInfo::exists(full))
                paths << full;
        }
    }
    return paths;
}

} // namespace

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
{
    m_player = new QMediaPlayer(this);
    m_output = new QAudioOutput(this);
    m_player->setAudioOutput(m_output);
    m_output->setVolume(m_volume / 100.0);
}

QString AudioPlayer::findAudio(const QString &key)
{
    const QStringList paths = candidatePaths(key);
    return paths.isEmpty() ? QString() : paths.first();
}

QString AudioPlayer::audioFolderHint()
{
    for (const QString &root : Artwork::resourceRoots()) {
        if (QFileInfo::exists(root + "audio"))
            return root + "audio/";
    }
    return QCoreApplication::applicationDirPath() + "/resources/audio/";
}

bool AudioPlayer::playBackground(const QString &key, int volumePercent)
{
    m_volume = qBound(0, volumePercent, 100);
    const QString path = findAudio(key);
    if (path.isEmpty()) {
        m_playing = false;
        return false;
    }

    m_currentKey = key;
    m_output->setVolume(m_volume / 100.0);
    m_player->setLoops(QMediaPlayer::Infinite);
    m_player->setSource(QUrl::fromLocalFile(path));
    m_player->play();
    m_playing = true;
    return true;
}

void AudioPlayer::stopBackground()
{
    m_player->stop();
    m_playing = false;
    m_suspended = false;
}

bool AudioPlayer::isBackgroundPlaying() const
{
    return m_playing;
}

qint64 AudioPlayer::backgroundPositionMs() const
{
    return m_player->position();
}

// 题目音频：用独立播放器播放，播完（或出错）后自动释放
void AudioPlayer::playEffect(const QString &key, int volumePercent)
{
    stopEffect(); // 同一时间只播放一段题目音频
    const QString path = findAudio(key);
    if (path.isEmpty())
        return;

    m_effect = new QMediaPlayer(this);
    auto *output = new QAudioOutput(this);
    m_effect->setAudioOutput(output);
    output->setVolume(qBound(0, volumePercent, 100) / 100.0);
    m_effect->setSource(QUrl::fromLocalFile(path));
    connect(m_effect, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus st) {
        if ((st == QMediaPlayer::EndOfMedia || st == QMediaPlayer::InvalidMedia) && m_effect) {
            m_effect->deleteLater();
            m_effect = nullptr;
        }
    });
    m_effect->play();
}

void AudioPlayer::stopEffect()
{
    if (m_effect) {
        m_effect->stop();
        m_effect->deleteLater();
        m_effect = nullptr;
    }
}

void AudioPlayer::suspendBackground()
{
    if (!m_playing) {
        m_suspended = false;
        return;
    }
    m_player->pause();
    m_suspended = true;
    m_playing = false;
}

void AudioPlayer::resumeBackground()
{
    if (!m_suspended)
        return;
    m_suspended = false;
    m_player->play();
    m_playing = true;
}
