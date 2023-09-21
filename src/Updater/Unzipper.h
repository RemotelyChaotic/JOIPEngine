#ifndef CUNZIPPER_H
#define CUNZIPPER_H

#include <QString>
#include <functional>

namespace zipper
{
  bool Unzip(const QString& sPathSource,
             std::function<void(const QString&)> fnMsg,
             std::function<void(qint32,qint32)> fnProgress,
             QString* psErr);
};

#endif // CUNZIPPER_H
