#pragma once

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QFile>
#include <QVBoxLayout>

#include "lyricfile/lyricfile.h"
#include "lyricprocesslabel/lyricprocesslabel.h"

class LyricWindow : public QMainWindow
{
   Q_OBJECT
public:
   explicit LyricWindow(QWidget *parent = nullptr);
   ~LyricWindow();
private:
   class LyricThread : public QThread {
public:
      LyricThread(LyricWindow *window);
      virtual void run();
      void interrupt();
      void readFromLyricFile(QFile& file);
      void clearLyric(QString filename);

private:
      LyricWindow *window;
      bool runnable;
      cszt::LyricFile lyricFile;
      QMutex runnableLock;
      QMutex lyricLock;
      QString filename;
   };

   QWidget *centralWidget;
   QVBoxLayout *gridLayout;
   LyricProcessLabel *mainLabel;
   QList<LyricProcessLabel *> otherLabels;
   LyricThread *thread;
};
