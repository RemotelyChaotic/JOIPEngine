#include "HelpFactory.h"

CHelpFactory::CHelpFactory() :
  CSystemBase()
{

}
CHelpFactory::~CHelpFactory()
{}

//----------------------------------------------------------------------------------------
//
void CHelpFactory::Initialize()
{
  SetInitialized(true);
}

//----------------------------------------------------------------------------------------
//
void CHelpFactory::Deinitialize()
{
  SetInitialized(false);
}
