#include "message.hpp"

enum Message {
  Lighting,
  Security,
};

// lighting handler handle the message from the lighting system
void *lighting_handler(void *arg) {}

void *security_handler(void *arg) {}
