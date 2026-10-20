#include "artwork.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QHash>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

namespace {

QHash<QString, QPixmap> g_cache;   // 缓存已生成的图，避免每次重绘都重新加载

const QColor kGold(212, 175, 55);
const QColor kGoldLight(240, 217, 140);

// 可执行文件所在目录
QString appDir()
{
    return QCoreApplication::applicationDirPath();
}

// 资源根目录候选：从可执行文件目录逐级向上找 resources/，再接工作目录
QStringList buildResourceRoots()
{
    QStringList roots;
    QDir dir(appDir());
    for (int level = 0; level < 5; ++level) {
        roots << dir.absolutePath() + "/resources/";
        if (!dir.cdUp())
            break;
    }
    roots << QDir::currentPath() + "/resources/";
    roots.removeDuplicates();
    return roots;
}

// 在候选路径里找到第一个存在的文件
QString findFile(const QStringList &relativeNames)
{
    const QStringList roots = buildResourceRoots();
    for (const QString &root : roots) {
        for (const QString &rel : relativeNames) {
            const QString full = root + rel;
            if (QFileInfo::exists(full))
                return full;
        }
    }
    return QString();
}

// 生成若干扩展名的候选文件名：base.png / base.jpg / ...
QStringList withExtensions(const QString &base)
{
    QStringList list;
    const QStringList exts = {"png", "jpg", "jpeg", "webp", "bmp"};
    for (const QString &e : exts)
        list << QString("%1.%2").arg(base, e);
    return list;
}

// 等比缩放铺满目标尺寸，再居中裁剪
QPixmap fitCover(const QPixmap &src, const QSize &size)
{
    if (src.isNull())
        return QPixmap();
    const QPixmap scaled =
        src.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    if (scaled.size() == size)
        return scaled;
    return scaled.copy(QRect((scaled.width() - size.width()) / 2,
                             (scaled.height() - size.height()) / 2, size.width(), size.height()));
}

// 底部标题条 + 文字
void drawTitleStrip(QPainter &painter, const QSize &size, const QString &title)
{
    if (title.isEmpty())
        return;
    const int w = size.width();
    const int h = size.height();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(12, 10, 12, 190));
    painter.drawRect(QRectF(0, h * 0.82, w, h * 0.18));

    // 字号自动缩小，保证标题放得下
    QFont font = painter.font();
    const int maxSize = qMax(12, static_cast<int>(h * 0.10));
    int used = 9;
    for (int s = maxSize; s >= 9; --s) {
        font.setPixelSize(s);
        font.setBold(true);
        if (QFontMetrics(font).horizontalAdvance(title) <= w - 12) {
            used = s;
            break;
        }
    }
    font.setPixelSize(used);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(kGoldLight);
    painter.drawText(QRect(6, static_cast<int>(h * 0.83), w - 12, static_cast<int>(h * 0.16)),
                     Qt::AlignCenter, title);
}

// 金色外边框
void drawGoldBorder(QPainter &painter, const QSize &size)
{
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(kGold, 2));
    painter.drawRect(QRectF(0.5, 0.5, size.width() - 1.0, size.height() - 1.0));
}

} // namespace

namespace Artwork {

QStringList resourceRoots()
{
    return buildResourceRoots();
}

QString findUserImage(const QStringList &relativeNames)
{
    return findFile(relativeNames);
}

//  主界面背景：用 resources/backgrounds/main_bg.*
QPixmap mainBackground(const QSize &size)
{
    const QSize safe(qMax(1, size.width()), qMax(1, size.height()));
    const QString key = QString("main|%1x%2").arg(safe.width()).arg(safe.height());
    if (g_cache.contains(key))
        return g_cache.value(key);

    QPixmap pm;
    const QString file = findUserImage(withExtensions("backgrounds/main_bg"));
    if (!file.isEmpty()) {
        const QPixmap raw(file);
        if (!raw.isNull()) {
            const int canvasW = qMax(safe.width(), safe.height() * 4 / 3);
            const QPixmap photo = fitCover(raw, QSize(canvasW, canvasW * 3 / 4)); // 4:3
            pm = fitCover(photo, safe);
            QPainter painter(&pm);
            painter.fillRect(QRect(QPoint(0, 0), safe), QColor(12, 8, 10, 110)); // 压暗
            painter.end();
        }
    }

    g_cache.insert(key, pm);
    return pm;
}

//  答题弹窗背景：用 resources/backgrounds/quiz_bg.*
QPixmap quizBackground(const QSize &size)
{
    const QSize safe(qMax(1, size.width()), qMax(1, size.height()));
    const QString key = QString("quiz|%1x%2").arg(safe.width()).arg(safe.height());
    if (g_cache.contains(key))
        return g_cache.value(key);

    QPixmap pm;
    const QString file = findUserImage(withExtensions("backgrounds/quiz_bg"));
    if (!file.isEmpty()) {
        const QPixmap raw(file);
        if (!raw.isNull()) {
            const int canvasW = qMax(safe.width(), safe.height() * 4 / 3);
            const QPixmap photo = fitCover(raw, QSize(canvasW, canvasW * 3 / 4)); // 4:3
            pm = fitCover(photo, safe);
            QPainter painter(&pm);
            painter.fillRect(QRect(QPoint(0, 0), safe), QColor(16, 8, 10, 140)); // 压暗
            painter.end();
        }
    }

    g_cache.insert(key, pm);
    return pm;
}

//  地标卡片：用 resources/places/<key>.*（如 jinci.jpg）
QPixmap placeImage(const QString &key, const QString &title, const QSize &size)
{
    const QSize safe(qMax(1, size.width()), qMax(1, size.height()));
    const QString cacheKey =
        QString("place|%1|%2x%3").arg(key, QString::number(safe.width()),
                                      QString::number(safe.height()));
    if (g_cache.contains(cacheKey))
        return g_cache.value(cacheKey);

    QPixmap pm;
    const QString file = findUserImage(withExtensions("places/" + key));
    if (!file.isEmpty()) {
        const QPixmap raw(file);
        if (!raw.isNull()) {
            pm = fitCover(raw, safe);
            QPainter painter(&pm);
            painter.setRenderHint(QPainter::Antialiasing, true);
            drawTitleStrip(painter, safe, title);
            drawGoldBorder(painter, safe);
            painter.end();
        }
    }

    g_cache.insert(cacheKey, pm);
    return pm;
}

//  戏曲图片题：用 resources/quiz/<key>.*（如 mask_red.jpg）
QPixmap quizImage(const QString &key, const QSize &size)
{
    const QSize safe(qMax(1, size.width()), qMax(1, size.height()));
    const QString cacheKey = QString("qi|%1|%2x%3").arg(key, QString::number(safe.width()),
                                                        QString::number(safe.height()));
    if (g_cache.contains(cacheKey))
        return g_cache.value(cacheKey);

    QPixmap pm;
    const QString file = findUserImage(withExtensions("quiz/" + key));
    if (!file.isEmpty()) {
        const QPixmap raw(file);
        if (!raw.isNull()) {
            pm = fitCover(raw, safe);
            QPainter painter(&pm);
            painter.setRenderHint(QPainter::Antialiasing, true);
            drawGoldBorder(painter, safe);
            painter.end();
        }
    }

    g_cache.insert(cacheKey, pm);
    return pm;
}

//  棋子：棋盘上显示玩家位置
void paintTokenFace(QPainter *painter, const QRectF &rect, const QColor &color)
{
    if (!painter)
        return;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(rect);

    const double w = rect.width();
    const double h = rect.height();
    painter->setBrush(QColor(255, 255, 255, 210));
    QPainterPath left;
    left.moveTo(rect.center().x() - w * 0.05, rect.center().y() - h * 0.06);
    left.cubicTo(rect.center().x() - w * 0.30, rect.center().y() - h * 0.26,
                 rect.center().x() - w * 0.36, rect.center().y() + h * 0.06,
                 rect.center().x() - w * 0.10, rect.center().y() - h * 0.02);
    left.closeSubpath();
    painter->drawPath(left);
    QPainterPath right;
    right.moveTo(rect.center().x() + w * 0.05, rect.center().y() - h * 0.06);
    right.cubicTo(rect.center().x() + w * 0.30, rect.center().y() - h * 0.26,
                  rect.center().x() + w * 0.36, rect.center().y() + h * 0.06,
                  rect.center().x() + w * 0.10, rect.center().y() - h * 0.02);
    right.closeSubpath();
    painter->drawPath(right);
    painter->restore();
}

} // namespace Artwork
