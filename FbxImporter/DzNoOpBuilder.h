#pragma once

#include "AbstractNodeBuilder.h"

class DzNoOpBuilder : public AbstractNodeBuilder {
 public:
  DzNoOpBuilder();
  ~DzNoOpBuilder();

  void buildNode();
};
