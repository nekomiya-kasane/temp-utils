#include "basic_signal.h"

void Connection::disconnect()
{
  if (!isValid()) {
    return;
  }
  disconnector_();
  disconnected_ = true;
}
