#pragma once

#include "domain/User.h"
#include "domain/Room.h"
#include "domain/Message.h"
#include <soci/soci.h>
#include <chrono>
namespace soci{
    template<>
    struct type_conversion<std::chrono::system_clock::time_point> {
        typedef std::tm base_type;

        static void from_base(const std::tm& t, indicator ind, std::chrono::system_clock::time_point& tp) {
            if (ind == i_ok) {
                std::tm temp_t = t;
                tp = std::chrono::system_clock::from_time_t(std::mktime(&temp_t));
            }
        }
        static void to_base(const std::chrono::system_clock::time_point& tp, std::tm& t, indicator& ind) {
            std::time_t tt = std::chrono::system_clock::to_time_t(tp);
            t = *std::localtime(&tt);
            ind = i_ok;
        }
    };
    template<>
    struct type_conversion<Message>{
        typedef values base_type;
        static void from_base(const values& v, indicator /*ind*/,Message& msg){
            msg.setId(v.get<long long>("id"));
            msg.setRoomId(v.get<long long>("room_id"));
            msg.setSenderId(v.get<long long>("sender_id"));
            msg.setContent(v.get<std::string>("content"));
            msg.setCreatedAt(v.get<std::chrono::system_clock::time_point>("created_at"));
        }
        static void to_base(const Message& msg, values& v, indicator& ind){
            v.set("room_id", msg.getRoomId());
            v.set("sender_id", msg.getSenderId());
            v.set("content", msg.getContent());
            ind = i_ok;
        }
    };
    template<>
    struct type_conversion<Room>{
        typedef values base_type;
        static void from_base(const values& v, indicator /*ind*/,Room& room){
            room.setId(v.get<long long>("id"));
            room.setName(v.get<std::string>("name"));
            room.setCreatorId(v.get<long long>("creator_id"));
            room.setCreatedAt(v.get<std::chrono::system_clock::time_point>("created_at"));
        }
        static void to_base(const Room& room, values& v, indicator& ind){
            v.set("name", room.getName());
            v.set("creator_id", room.getCreatorId());
            ind = i_ok;
        }
    };
    template<>
    struct type_conversion<User>{
        typedef values base_type;
        static void from_base(const values& v, indicator /*ind*/,User& user){
            user.setId(v.get<long long>("id"));
            user.setUsername(v.get<std::string>("username"));
            user.setHashedPassword(v.get<std::string>("hashed_password"));
            user.setSalt(v.get<std::string>("salt"));
            user.setCreatedAt(v.get<std::chrono::system_clock::time_point>("created_at"));
        }
        static void to_base(const User& user, values& v, indicator& ind){
            v.set("username", user.getUsername());
            v.set("hashed_password", user.getHashedPassword());
            v.set("salt", user.getSalt());
            ind = i_ok;
        }
    };
}