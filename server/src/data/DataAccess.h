#pragma once


#include "domain/User.h"
#include "domain/Room.h"
#include "domain/Message.h"


#include "data/SociTypeConversion.h"


#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h> // 如果其他地方也需要


#include "data/ConnectionPool.h"
#include "data/IUserRepository.h"
#include "data/IMessageRepository.h"
#include "data/IRoomRepository.h"
