#include "pch.h"

#include "listwindow.h"
#include "ui_listwindow.h"
#include "lyricwindow.h"

#include <QStringListModel>
#include <QDir>
#include <QUrl>
#include <QMediaPlayer>
#include <QCloseEvent>
#include <QIcon>
#include <QMenu>
#include <QtWin>
#include <QWinThumbnailToolBar>
#include <QProcess>
#include <QMessageBox>
#include <QMenuBar>

extern QMediaPlayer *mediaPlayer;
extern LyricWindow  *lyricWindow;

extern QString path2name(QString);
extern QString readableFileName(QString fileName);

static bool changing = false;

ListWindow::ListWindow(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::ListWindow)
{
   ui->setupUi(this);
   ui->playButton->setShortcut(QKeySequence("Space"));
   setWindowTitle("音乐播放器");

   QMenuBar *menuBar     = this->menuBar();
   QMenu    *controlMenu = menuBar->addMenu("控制");
   QMenu    *windowMenu  = menuBar->addMenu("窗口");
   QMenu    *aboutMenu   = menuBar->addMenu("关于");
   connect(controlMenu->addAction("播放"), &QAction::triggered, [] {
      mediaPlayer->play();
   });
   connect(controlMenu->addAction("重新播放"), &QAction::triggered, [] {
      mediaPlayer->setPosition(0);
      mediaPlayer->play();
   });
   connect(controlMenu->addAction("暂停"), &QAction::triggered, [] {
      mediaPlayer->pause();
   });
   QAction *nextMusic = controlMenu->addAction("下一曲");
   connect(nextMusic, &QAction::triggered, [this] {
      randomPlay();
   });
   controlMenu->addSeparator();
   repeatAction = controlMenu->addAction("洗脑循环");
   repeatAction->setCheckable(true);
   lyricWindowAction = windowMenu->addAction("桌面歌词");
   lyricWindowAction->setCheckable(true);
   lyricWindowAction->setChecked(true);
   connect(lyricWindowAction, &QAction::triggered, [this] {
      bool checked = lyricWindowAction->isChecked();
      if (checked)
      {
         lyricTrayAction->setText("隐藏桌面歌词");
         lyricWindow->show();
      }
      else
      {
         lyricTrayAction->setText("显示桌面歌词");
         lyricWindow->hide();
      }
   });
   windowMenu->addSeparator();
   connect(windowMenu->addAction("刷新列表"), &QAction::triggered, [this] {
      loadMusicList();
   });

   connect(aboutMenu->addAction("打开 Java 版播放器"), &QAction::triggered, [this] {
      int option = QMessageBox::question(this, "打开 Java 版播放器", "Java 版播放器已经较长时间没有维护，不支持高级歌词格式，占用电脑资源较大，且需要电脑已安装 JRE 1.8 或以上版本并配置环境变量。\n\n真的要打开 Java 版播放器吗？");
      if (option == QMessageBox::Yes)
      {
         QStringList arguments;
         arguments << "-jar";
         arguments << "music.jar";
         QProcess process;
         process.setProgram("java");
         process.setArguments(arguments);
         if (!process.startDetached())
         {
            QApplication::beep();
            QMessageBox::critical(this, "启动失败", "无法启动 Java 版播放器，请确认电脑已安装 JRE 1.8 或以上版本，并成功配置环境变量。");
         }
      }
   });

   connect(aboutMenu->addAction("关于 Qt"), &QAction::triggered, [this] {
      QMessageBox::aboutQt(this, "关于 Qt");
   });

   // 任务栏磁贴按钮
   QMenu *menu = new QMenu(this);
   if (QtWin::isCompositionEnabled())
   {
      QtWin::enableBlurBehindWindow(menu);
   }
   else
   {
      QtWin::disableBlurBehindWindow(menu);
      menu->hide();
   }

   QWinThumbnailToolBar *taskBar = new QWinThumbnailToolBar(this);
   taskBar->setWindow(windowHandle());

   QWinThumbnailToolButton *prevToolButton = new QWinThumbnailToolButton(taskBar);
   prevToolButton->setToolTip("上一曲");
   prevToolButton->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MediaSkipBackward));
   prevToolButton->setEnabled(false);

   playToolButton = new QWinThumbnailToolButton(taskBar);
   playToolButton->setToolTip("播放");
   playToolButton->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MediaPlay));
   connect(playToolButton, &QWinThumbnailToolButton::clicked, ui->playButton, &QPushButton::click);

   QWinThumbnailToolButton *nextToolButton = new QWinThumbnailToolButton(taskBar);
   nextToolButton->setToolTip("下一曲");
   nextToolButton->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MediaSkipForward));
   connect(nextToolButton, &QWinThumbnailToolButton::clicked, nextMusic, &QAction::trigger);

   taskBar->addButton(prevToolButton);
   taskBar->addButton(playToolButton);
   taskBar->addButton(nextToolButton);

   // 系统托盘菜单
   systemTrayMenu = new QMenu();
   connect(systemTrayMenu->addAction("打开主菜单"), &QAction::triggered, this, &ListWindow::reopen);
   systemTrayMenu->addSeparator();
   playTrayAction = systemTrayMenu->addAction("播放");
   connect(playTrayAction, &QAction::triggered, ui->playButton, &QPushButton::click);
   connect(systemTrayMenu->addAction("下一曲"), &QAction::triggered, nextMusic, &QAction::trigger);
   systemTrayMenu->addSeparator();
   lyricTrayAction = systemTrayMenu->addAction("隐藏桌面歌词");
   connect(lyricTrayAction, &QAction::triggered, lyricWindowAction, &QAction::trigger);
   systemTrayMenu->addSeparator();
   connect(systemTrayMenu->addAction("退出"), &QAction::triggered, [] {
      QApplication::quit();
   });

   // 绑定UI事件
   connect(ui->playButton, &QPushButton::clicked, [this] {
      QMediaPlayer::State state = mediaPlayer->state();
      if (state == QMediaPlayer::State::PlayingState)
      {
         mediaPlayer->pause();
         return;
      }
      else if (state == QMediaPlayer::State::PausedState)
      {
         mediaPlayer->play();
         return;
      }
      QModelIndex currentIndex = this->ui->musicList->currentIndex();
      if (currentIndex.isValid())
      {
         QString musicName = this->musicList.at(currentIndex.row());
         mediaPlayer->setMedia(QUrl::fromLocalFile(MUSIC_DIR + musicName));
         mediaPlayer->play();
      }
      else
      {
         this->randomPlay();
      }
   });
   connect(ui->musicList, &QListView::clicked, [this](QModelIndex index) {
      if (index.isValid())
      {
         changing          = true;
         QString musicName = this->musicList.at(index.row());
         mediaPlayer->setMedia(QUrl::fromLocalFile(MUSIC_DIR + musicName));
         mediaPlayer->play();
      }
   });
   connect(ui->musicList, &QListView::customContextMenuRequested, [this] {
      QModelIndex currentIndex = ui->musicList->currentIndex();
      if (currentIndex.isValid())
      {
         QMenu listMenu(ui->musicList);
         connect(listMenu.addAction("调整歌词"), &QAction::triggered, [this, currentIndex] {
            QString musicFileName = this->musicList.at(currentIndex.row());
            QString musicName     = readableFileName(musicFileName);
            QString music         = QDir(MUSIC_DIR).absolutePath() + "/" + musicFileName;
            QString lyric         = QDir(LRC_DIR).absolutePath() + "/" + musicName + ".lrc";
            // 启动另外的程序编辑歌词
            QProcess process;
            QStringList arguments;
            arguments << music;
            if (QFile(lyric).exists())
            {
               arguments << lyric;
            }
            process.setProgram("AdjustMusic.exe");
            process.setArguments(arguments);
            // 使用 startDetached() 分离进程
            bool started = process.startDetached();
            if (!started)
            {
               QApplication::beep();
               QMessageBox::critical(this, "启动失败", "无法启动应用程序 AdjustMusic.exe，重新安装软件可能会解决此问题。");
            }
         });
         listMenu.exec(QCursor::pos());
      }
   });

   // 绑定播放器事件
   connect(mediaPlayer, &QMediaPlayer::stateChanged, [this](QMediaPlayer::State state) {
      switch (state)
      {
      case QMediaPlayer::State::StoppedState:
         if (changing)
         {
            changing = false;
            break;
         }
         if (repeatAction->isChecked())
         {
            mediaPlayer->setMedia(mediaPlayer->media());
            mediaPlayer->play();
         }
         else
         {
            this->randomPlay();
         }
         break;

      case QMediaPlayer::State::PausedState:
         this->ui->playButton->setText("播放");
         this->playTrayAction->setText("播放");
         this->playToolButton->setToolTip("播放");
         this->playToolButton->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MediaPlay));
         break;

      case QMediaPlayer::State::PlayingState:
         this->ui->playButton->setText("暂停");
         this->playTrayAction->setText("暂停");
         this->playToolButton->setToolTip("暂停");
         this->playToolButton->setIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_MediaPause));
         changing = false;
         break;
      }
   });
   connect(mediaPlayer, &QMediaPlayer::durationChanged, [this](qint64 duration) {
      qint64 hour  = duration / 60 / 60 / 1000;
      qint8 minute = duration / 60 / 1000 % 60;
      qint8 second = duration / 1000 % 60;
      QString time = QString("%1:%2:%3").arg(hour).arg(minute, 2, 10, QLatin1Char('0')).arg(second, 2, 10, QLatin1Char('0'));
      ui->totalTime->setText(time);
      ui->progressBar->setRange(0, static_cast<int>(duration));
   });
   connect(mediaPlayer, &QMediaPlayer::positionChanged, [this](qint64 position) {
      qint64 hour  = position / 60 / 60 / 1000;
      qint8 minute = position / 60 / 1000 % 60;
      qint8 second = position / 1000 % 60;
      QString time = QString("%1:%2:%3").arg(hour).arg(minute, 2, 10, QLatin1Char('0')).arg(second, 2, 10, QLatin1Char('0'));
      ui->currentTime->setText(time);
      ui->progressBar->setValue(static_cast<int>(position));
   });
   connect(mediaPlayer, &QMediaPlayer::mediaChanged, [this](QMediaContent media) {
      if (media.isNull())
      {
         this->ui->nowPlaying->setText(nullptr);
         systemTray->setToolTip("已停止");
      }
      else
      {
         QString playing = QString("正在播放：") + path2name(media.canonicalUrl().path());
         this->ui->nowPlaying->setText(playing);
         if (systemTray)
         {
            systemTray->setToolTip(playing);
         }
      }
   });
   // 更新列表
   loadMusicList();

   systemTray = nullptr;
}


ListWindow::~ListWindow()
{
   delete ui;
   if (systemTray)
   {
      systemTray->deleteLater();
   }
   if (systemTrayMenu)
   {
      systemTrayMenu->deleteLater();
   }
}


void ListWindow::loadMusicList()
{
   QDir        dir(MUSIC_DIR);
   QStringList filterList;

   filterList << "*.wav" << "*.mp3" << "*.ogg";
   this->musicList = dir.entryList(filterList, QDir::Files | QDir::Readable, QDir::Name);

   QAbstractItemModel *model = ui->musicList->model();
   if (model)
   {
      model->deleteLater();
   }
   ui->musicList->setModel(new QStringListModel(this->musicList));
}


void ListWindow::randomPlay()
{
   // 初始化随机种子
   time_t tm;

   time(&tm);
   srand(static_cast<unsigned int>(tm));
   // 随机选择一首
   int size = this->musicList.size();
   if (size == 0)
   {
      return;
   }
   int     index     = rand() % size;
   QString musicName = this->musicList.at(index);
   // 播放
   mediaPlayer->setMedia(QUrl::fromLocalFile(MUSIC_DIR + musicName));
   mediaPlayer->play();
}


void ListWindow::closeEvent(QCloseEvent *event)
{
   if (mediaPlayer->state() != QMediaPlayer::PlayingState)
   {
      event->accept();
      QApplication::quit();
      return;
   }
   event->ignore();
   hide();

   if (systemTray == nullptr)
   {
      systemTray = new QSystemTrayIcon(this);
      systemTray->setContextMenu(systemTrayMenu);
      QIcon icon(ICON);
      systemTray->setIcon(icon);
      QString playing = QString("正在播放：") + path2name(mediaPlayer->media().canonicalUrl().path());
      systemTray->setToolTip(playing);
      QObject::connect(systemTray, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason) {
         if (reason == QSystemTrayIcon::ActivationReason::DoubleClick)
         {
            reopen();
         }
      });
      QObject::connect(systemTray, &QSystemTrayIcon::messageClicked, this, &ListWindow::reopen);
   }

   systemTray->show();
   systemTray->showMessage("已最小化到托盘", "播放器将在后台持续播放音乐。若要退出程序，请先暂停播放后再点击关闭按钮。");
}


void ListWindow::reopen()
{
   setWindowState(windowState() & ~Qt::WindowMinimized);
   show();
   QApplication::alert(this);
   if (systemTray)
   {
      systemTray->hide();
   }
}
