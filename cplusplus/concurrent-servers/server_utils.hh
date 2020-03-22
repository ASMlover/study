#pragma once

#include "common_utils.hh"

namespace svrutils {

void serve_connection(common::UniqueSocket& sockfd);

}