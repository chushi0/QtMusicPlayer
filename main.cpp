#include "pch.h"

#include "listwindow.h"
#include "lyricwindow.h"

#include <QApplication>
#include <QSharedMemory>

#include <QMediaPlayer>

extern QMediaPlayer *mediaPlayer;
extern ListWindow   *listWindow;
extern LyricWindow  *lyricWindow;

QMediaPlayer *mediaPlayer;
ListWindow   *listWindow;
LyricWindow  *lyricWindow;

int main(int argc, char *argv[])
{
   QSharedMemory memory;

   memory.setKey(MEMORY_KEY);
   if (memory.attach())
   {
      memory.lock();
      qint8 *data = static_cast<qint8 *>(memory.data());
      data[0] = 1;
      memory.unlock();
      return 0;
   }
   if (memory.create(1))
   {
      memory.lock();
      qint8 *data = static_cast<qint8 *>(memory.data());
      data[0] = 0;
      memory.unlock();

      QApplication a(argc, argv);
      a.addLibraryPath("./lib/");

      QMediaPlayer m;
      mediaPlayer = &m;

      ListWindow  w;
      LyricWindow lw;
      listWindow  = &w;
      lyricWindow = &lw;
      w.show();
      lw.show();

      return a.exec();
   }
   return 0;
}
