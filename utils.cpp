#include <QString>

QString path2name(QString path)
{
   int splashIndex = path.lastIndexOf('/');

   if (splashIndex == -1)
   {
      splashIndex = path.lastIndexOf('\\');
   }
   if (splashIndex == -1)
   {
      return path;
   }
   return path.mid(splashIndex + 1);
}


QString readableFileName(QString fileName)
{
   fileName = path2name(fileName);
   int dotIndex = fileName.lastIndexOf('.');

   return fileName.mid(0, dotIndex);
}
