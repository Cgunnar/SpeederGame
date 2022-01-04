#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define NOMINMAX
#include <Windows.h>

#include <wrl.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <debugapi.h>

#include <d3dcompiler.h>

#include <cmath>

#include <assert.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <filesystem>

#include <algorithm>

#include <chrono>
#include <memory>
#include <functional>
#include <optional>
#include <variant>

#include <string>
#include <vector>
#include <array>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <tuple>

#include <mutex>
#include <thread>
#include <atomic>
#include <semaphore>
#include <future>