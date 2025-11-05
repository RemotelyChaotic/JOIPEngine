#ifndef CDATABASEDATA_H
#define CDATABASEDATA_H

#include "Kink.h"

#include <QMutex>

#include <memory>
#include <vector>

struct SProject;
typedef std::shared_ptr<SProject>       tspProject;
typedef std::vector<tspProject>         tvspProject;

class CDatabaseData : public QMutex
{
public:
  CDatabaseData();
  ~CDatabaseData() {}

  tvspProject                            m_vspProjectDatabase;
  tKinkKategories                        m_kinkKategoryMap;
};

#endif // CDATABASEDATA_H
