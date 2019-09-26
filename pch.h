#pragma once

// 全局常量

#define MEMORY_KEY    "cszt0_MediaPlayer"

#define MUSIC_DIR     "./music/"
#define LRC_DIR       "./lrc/"
#define ICON          "./icon.ico"


// 基本函数
template<class T>
inline const T& max(const T& t1, const T& t2)
{
   return t1 > t2 ? t1 : t2;
}
