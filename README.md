# QtMusicPlayer
基于Qt实现的音乐播放器
![](https://img.shields.io/badge/Build-passing-brightgreen) ![](https://img.shields.io/badge/Qt-5.13.0-brightgreen) ![](https://img.shields.io/badge/License-LGPL-brightgreen) ![](https://img.shields.io/badge/License-anti_icu-brightgreen)
## 功能
1. 随机播放歌曲，并有歌词与进度显示
2. 可以指定立即播放的歌曲
3. 可设置洗脑循环
4. 关闭后可以切换到后台运行

## 依赖
1. [QtMusicPlayer_MusicLyric](https://github.com/chushi0/QtMusicPlayer_MusicLyric)
2. [QtMusicPlayer_LyricProcessLabel](https://github.com/chushi0/QtMusicPlayer_LyricProcessLabel)

## 部署
1. 修改[MusicPlayer.pro](https://github.com/chushi0/QtMusicPlayer/blob/master/MusicPlayer.pro)文件，正确引入上述两个依赖库
2. 使用`qmake`和`cmake`命令编译
3. 将MusicPlayer.exe复制到单独文件夹，使用`windeployqt`命令导出依赖库
4. 将上述两个依赖库的dll文件复制到MusicPlayer.exe所在文件夹
5. 将文件[icon.ico](https://github.com/chushi0/QtMusicPlayer/blob/master/icon.ico)复制到MusicPlayer.exe所在文件夹
6. 将项目[MusicWithLrc](https://github.com/chushi0/MusicWithLrc)导出为music.jar，复制到MusicPlayer.exe所在文件夹
7. 将项目[QtMusicPlayer_AdjustMusic](https://github.com/chushi0/QtMusicPlayer_AdjustMusic)导出为AdjustMusic.exe，复制到MusicPlayer.exe所在文件夹
8. 在MusicPlayer.exe所在文件夹中创建music和lrc文件夹。其中music文件夹存放音乐，lrc文件夹存放歌词
9. 运行MusicPlayer.exe
