#pragma once

// --- 关键：第一步，包含所有实体类 ---
// 这确保了在包含 SOCI 之前，所有自定义类型都已经是已知的
#include "domain/User.h"
#include "domain/Room.h"
#include "domain/Message.h"

// --- 关键：第二步，包含我们自己的类型转换定义 ---
// 这确保了在 SOCI 尝试实例化任何模板之前，
// 我们为自定义类型准备的“教科书”就已经被编译器读入了。
#include "data/SociTypeConversion.h"

// --- 关键：第三步，最后才包含 SOCI 的核心头文件 ---
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h> // 如果其他地方也需要

// --- 第四步 (可选)：包含其他数据层组件 ---
#include "data/ConnectionPool.h"
#include "data/IUserRepository.h"
#include "data/IMessageRepository.h"
#include "data/IRoomRepository.h"