#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QWinThumbnailToolButton>

namespace Ui {
class ListWindow;
}

class ListWindow : public QMainWindow
{
   Q_OBJECT

public:
   explicit ListWindow(QWidget *parent = nullptr);
   ~ListWindow();
   void closeEvent(QCloseEvent *event);

public Q_SLOTS:
   void reopen();

private:

   QStringList musicList;
   Ui::ListWindow *ui;
   QSystemTrayIcon *systemTray;
   QMenu *systemTrayMenu;

   // 以下指针变量无需在析构函数中释放
   QWinThumbnailToolButton *playToolButton;
   QAction *playTrayAction, *lyricTrayAction;
   QAction *repeatAction, *lyricWindowAction;

   void loadMusicList();
   void randomPlay();
};
