#include "pch.h"

#include "listwindow.h"
#include "lyricwindow.h"

#include <QMediaPlayer>
#include <QSharedMemory>

using cszt::LyricFile;
using cszt::Lyric;

extern QMediaPlayer *mediaPlayer;
extern ListWindow   *listWindow;

extern QString path2name(QString);
extern QString readableFileName(QString fileName);

void generateSize(int need, QList<LyricProcessLabel *>& container, QWidget *widget, QVBoxLayout *layout)
{
   int size = container.size();

   if (need > size)
   {
      for (int i = size; i < need; i++)
      {
         LyricProcessLabel *label = new LyricProcessLabel(widget);
         QFont             font;
         font.setPixelSize(24);
         label->setFont(font);
         label->setTextAndProgress("", 0, 0, 0);
         layout->insertWidget(layout->count() - 1, label, 0, Qt::AlignmentFlag::AlignTop);
         container << label;
      }
   }
}


inline void setAllEmpty(QList<LyricProcessLabel *>& labels)
{
   for (LyricProcessLabel *label:labels)
   {
      label->setTextAndProgress("", 0, 0, 0);
   }
}


LyricWindow::LyricWindow(QWidget *parent) : QMainWindow(parent)
{
   setObjectName("桌面歌词");
   setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
   setWindowOpacity(1);
   setAttribute(Qt::WA_TranslucentBackground);
   setAttribute(Qt::WA_TransparentForMouseEvents, true);
   // 设置控件
   resize(1920, 1080);
   centralWidget = new QWidget(this);
   centralWidget->setObjectName("centralWidget");
   gridLayout = new QVBoxLayout(centralWidget);
   gridLayout->setSpacing(6);
   gridLayout->setContentsMargins(11, 11, 11, 11);
   gridLayout->setObjectName("gridLayout");
   mainLabel = new LyricProcessLabel(centralWidget);
   mainLabel->setObjectName("mainLabel");
   QFont mainFont;
   mainFont.setPixelSize(32);
   mainLabel->setFont(mainFont);
   mainLabel->setTextAndProgress("", 0, 0, 0);
   gridLayout->addWidget(mainLabel, 0, Qt::AlignmentFlag::AlignTop);
   gridLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
   setCentralWidget(centralWidget);
   // 播放器事件
   connect(mediaPlayer, &QMediaPlayer::mediaChanged, [this](QMediaContent media) {
      QString musicName = readableFileName(path2name(media.canonicalUrl().path()));
      if (!media.isNull())
      {
         QFile file(LRC_DIR + musicName + ".lrc");
         thread->readFromLyricFile(file);
      }
      else
      {
         thread->clearLyric(musicName);
      }
   });
   connect(mediaPlayer, &QMediaPlayer::positionChanged, [this] {
      mainLabel->repaint();
      for (LyricProcessLabel *label : otherLabels)
      {
         label->repaint();
      }
   });

   // 线程
   thread = new LyricThread(this);
   thread->start();
}


LyricWindow::~LyricWindow()
{
   thread->interrupt();
   thread->wait();
   delete thread;
}


LyricWindow::LyricThread::LyricThread(LyricWindow *window) : QThread(), window(window)
{
   runnable = true;
}


void LyricWindow::LyricThread::interrupt()
{
   QMutexLocker locker(&runnableLock);

   runnable = false;
}


void LyricWindow::LyricThread::readFromLyricFile(QFile& file)
{
   if (file.exists())
   {
      QMutexLocker locker(&lyricLock);
      if (lyricFile << file)
      {
         filename = readableFileName(file.fileName());
         int need = lyricFile.getMaxOtherSize();
         generateSize(need, window->otherLabels, window->centralWidget, window->gridLayout);
      }
      else
      {
         clearLyric(readableFileName(file.fileName()));
      }
   }
   else
   {
      clearLyric(readableFileName(file.fileName()));
   }
}


void LyricWindow::LyricThread::clearLyric(QString filename)
{
   QMutexLocker locker(&lyricLock);

   lyricFile.clear();
   this->filename = filename;
}


void LyricWindow::LyricThread::run()
{
   QSharedMemory       sharedMemory;
   QMediaPlayer::State mediaState;

   Lyric lyric;

   sharedMemory.setKey(MEMORY_KEY);
   sharedMemory.attach();
   qint8 *data = static_cast<qint8 *>(sharedMemory.data());

   while (1)
   {
      sharedMemory.lock();
      if (data[0] == 1)
      {
         data[0] = 0;
         listWindow->reopen();
      }
      sharedMemory.unlock();
      mediaState = mediaPlayer->state();
      if (mediaState == QMediaPlayer::State::PlayingState)
      {
         bool valid;
         {
            QMutexLocker locker(&lyricLock);
            valid = lyricFile.lyricByTime(lyric, static_cast<long>(mediaPlayer->position()));
         }
         setAllEmpty(window->otherLabels);
         if (valid)
         {
            window->mainLabel->setTextAndProgress(lyric.mainText, lyric.mainTextBreakStart, lyric.mainTextBreakEnd, lyric.mainTextBreakProgress);
            int size = lyric.texts.size();
            for (int i = 0; i < size; i++)
            {
               window->otherLabels[i]->setTextAndProgress(lyric.texts[i], lyric.textBreakStarts[i], lyric.textBreakEnds[i], lyric.textBreakProgresses[i]);
            }
         }
         else
         {
            window->mainLabel->setTextAndProgress(filename, 0, filename.size(), lyric.mainTextBreakProgress);
         }
      }
      else if (mediaState == QMediaPlayer::State::StoppedState)
      {
         window->mainLabel->setTextAndProgress("", 0, 0, 0);
         setAllEmpty(window->otherLabels);
      }
      msleep(50);
      {
         QMutexLocker locker(&runnableLock);
         if (!runnable)
         {
            return;
         }
      }
   }
}
