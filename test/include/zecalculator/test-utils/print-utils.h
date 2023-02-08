#pragma once

#include <sstream>

#include <zecalculator/utils/token.h>

namespace zc {

std::ostream& operator << (std::stringstream& os, const Token& token);

}