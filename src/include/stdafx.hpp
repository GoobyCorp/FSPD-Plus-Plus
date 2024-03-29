#pragma once

#include <memory>
#include <string>
#include <stdio.h>
#include <cstdarg>
#include <memory.h>
#include <stdlib.h>
#include <filesystem>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/limits.h>
#include <experimental/random>

using namespace std;
namespace fs = std::filesystem;
using namespace std::experimental;

// internal
#include "bswap.hpp"
#include "types.hpp"
#include "memory.hpp"
#include "globals.hpp"
#include "cache.hpp"
#include "utils.hpp"
#include "fsp.hpp"