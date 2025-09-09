#pragma once

#include "domain/User.h"
#include "domain/Room.h"
#include "domain/Message.h"
#include <soci/soci.h>
#include <chrono>
#include <iostream>

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
            try{
                int id_val = v.get<int>("id");
                int room_id_val = v.get<int>("room_id");
                int sender_id_val = v.get<int>("sender_id");
				msg.setId(static_cast<long long>(id_val));
				msg.setRoomId(static_cast<long long>(room_id_val));
				msg.setSenderId(static_cast<long long>(sender_id_val));
				msg.setContent(v.get<std::string>("content"));
                // 3. �� TIMESTAMP ��Ϊ string ��ȡ��Ȼ���ֶ�����
                if (v.get_indicator("created_at") == i_ok) {
                    try {
                        // MySQL �� TIMESTAMP ������ std::tm ��ʽ����
                        std::tm tm = v.get<std::tm>("created_at");
                        msg.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
                    }
                    catch (...) {
                        // ���ʧ�ܣ�������Ϊ�ַ���
                        std::string created_at_str = v.get<std::string>("created_at");
                        std::tm tm = {};
                        if (sscanf(created_at_str.c_str(), "%d-%d-%d %d:%d:%d",
                            &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                            &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
                            tm.tm_year -= 1900;
                            tm.tm_mon -= 1;
                            msg.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
                        }
                        else {
                            // �������ʧ�ܣ�ʹ�õ�ǰʱ��
                            msg.setCreatedAt(std::chrono::system_clock::now());
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "!!! Exception in from_base: " << e.what() << std::endl;
                throw;
            }
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
            try {
                // ��ȫ��ȡ id
                if (v.get_indicator("id") == i_ok) {
                    room.setId(v.get<long long>("id"));
                }
                else {
                    std::cerr << "[WARNING] Column 'id' not found or is NULL!" << std::endl;
                }

                // ��ȫ��ȡ name
                if (v.get_indicator("name") == i_ok) {
                    room.setName(v.get<std::string>("name"));
                }
                else {
                    std::cerr << "[WARNING] Column 'name' not found or is NULL!" << std::endl;
                }

                // ��ȫ��ȡ creator_id
                if (v.get_indicator("creator_id") == i_ok) {
                    room.setCreatorId(v.get<long long>("creator_id"));
                }
                else {
                    std::cerr << "[WARNING] Column 'creator_id' not found or is NULL!" << std::endl;
                }
                // created_at ������ NULL����Ҫ���
                if (v.get_indicator("created_at") == i_ok) {
                    try {
                        // MySQL �� TIMESTAMP ������ std::tm ��ʽ����
                        std::tm tm = v.get<std::tm>("created_at");
                        room.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
                    }
                    catch (...) {
                        // ���ʧ�ܣ�������Ϊ�ַ���
                        std::string created_at_str = v.get<std::string>("created_at");
                        std::tm tm = {};
                        if (sscanf(created_at_str.c_str(), "%d-%d-%d %d:%d:%d",
                            &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                            &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
                            tm.tm_year -= 1900;
                            tm.tm_mon -= 1;
                            room.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
                        }
                        else {
                            // �������ʧ�ܣ�ʹ�õ�ǰʱ��
                            room.setCreatedAt(std::chrono::system_clock::now());
                        }
                    }
                }
                else {
                    // created_at �� NULL��ʹ�õ�ǰʱ��
                    room.setCreatedAt(std::chrono::system_clock::now());
                }
            }
            catch (const std::exception& e) {
                std::cerr << "!!! Exception in from_base: " << e.what() << std::endl;
                throw;
            }
        }
        static void to_base(const Room& room, values& v, indicator& ind){
            v.set("name", room.getName());
            v.set("creator_id", room.getCreatorId());
            ind = i_ok;
        }
    };
    template<>
    struct type_conversion<User> {
        typedef values base_type;

        static void from_base(const values& v, indicator /*ind*/, User& user) {
            try {
                // id �� int������ long long
                int id_val = v.get<int>("id");
                user.setId(static_cast<long long>(id_val));
                user.setUsername(v.get<std::string>("username"));
                user.setHashedPassword(v.get<std::string>("hashed_password"));
                user.setSalt(v.get<std::string>("salt"));

                // created_at ������ NULL����Ҫ���
                if (v.get_indicator("created_at") == i_ok) {
                    try {
                        // MySQL �� TIMESTAMP ������ std::tm ��ʽ����
                        std::tm tm = v.get<std::tm>("created_at");
                        user.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
                    }
                    catch (...) {
                        // ���ʧ�ܣ�������Ϊ�ַ���
                        std::string created_at_str = v.get<std::string>("created_at");
                        std::tm tm = {};
                        if (sscanf(created_at_str.c_str(), "%d-%d-%d %d:%d:%d",
                            &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                            &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
                            tm.tm_year -= 1900;
                            tm.tm_mon -= 1;
                            user.setCreatedAt(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
                        }
                        else {
                            // �������ʧ�ܣ�ʹ�õ�ǰʱ��
                            user.setCreatedAt(std::chrono::system_clock::now());
                        }
                    }
                }
                else {
                    // created_at �� NULL��ʹ�õ�ǰʱ��
                    user.setCreatedAt(std::chrono::system_clock::now());
                }
            }
            catch (const std::exception& e) {
                std::cerr << "!!! Exception in from_base: " << e.what() << std::endl;
                throw;
            }
        }

        static void to_base(const User& user, values& v, indicator& ind) {
            v.set("username", user.getUsername());
            v.set("hashed_password", user.getHashedPassword());
            v.set("salt", user.getSalt());

            // ת��ʱ��Ϊ�ַ���
            /*auto time_t_val = std::chrono::system_clock::to_time_t(user.getCreatedAt());
            std::tm* tm = std::localtime(&time_t_val);
            char buffer[20];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
            v.set("created_at", std::string(buffer));*/

            ind = i_ok;
        }
    };
}
