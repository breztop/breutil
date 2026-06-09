#pragma once

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <source_location>
#include <string_view>

#include "singleton.hpp"

/*


1.用于找到代码整体运行哪一个部分耗时
使用方式：
Hit：使用单例的时候，每次在需要测试的地方Hit，这一个Hit将会和上一个Hit之间的时间差打印出来,
    如果设置需要设置线性，即所有的Hit次数应该是一致的
原理： 比如在A.cpp B.cpp C.cpp D.cpp中都有一个Hit
    由于多线程的原因, 所以可能会出现 A的hit运行两次，B的hit才运行，
    实际运行A->A->B, 所以本来是第一个A和B之间的时间差，但是实际上是第二个A和B之间的时间差
    所以增加map记录是否存在和次数，List记录Hit应该的顺序

Interval：使用单例的时候，每次在需要测试的地方Interval，对于这一个Interval两次的时间差打印出来

2.静态函数mesture测试当前代码块的耗时
使用方式：TimeTest::mesture([]{
    // code block
}, true, "code block name");
*/
namespace bre {


class TimeTest : public Singleton<TimeTest> {
    friend class Singleton<TimeTest>;

    struct HitInfo {
        std::string msg;
        std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> time_vec;
        int count;
    };

public:
    static float Now() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch())
                   .count() /
               1000.0f;
    }

    static float PrintNow(std::string_view msg = "") {
        auto now = Now();
        // 最多3位小数
        std::cout.precision(3);
        std::cout << std::fixed << msg << " time: " << now << " ms" << std::endl;
        return now;
    }


    template <typename Func>
    static void Measure(Func func, bool print = false, std::string_view sv = "") {
        if (!print) {
            func();
            return;
        }

        auto s = std::chrono::high_resolution_clock::now();
        func();
        auto e = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> elapsed = e - s;
        std::cout << sv << "Spend: " << elapsed.count() << " ms\n";
    }

    void Interval(std::string msg, bool is_print = true,
                  std::source_location loc = std::source_location::current()) {
        auto key = std::string(loc.file_name()) + std::string(loc.function_name()) +
                   std::to_string(loc.line()) + std::to_string(loc.column()) + msg;

        auto cur_time = std::chrono::high_resolution_clock::now();

        if (m_intevalMap.find(key) == m_intevalMap.end()) {
            m_intevalMap[key] = cur_time;
            return;
        }

        auto pre_time = m_intevalMap[key];
        auto time_diff = cur_time - pre_time;

        if (is_print) {
            std::string out_msg = "Interval: " + msg + " Spend: ";
            if (m_time_unit == "ms") {
                auto elapsed = std::chrono::duration<double, std::milli>(time_diff).count();
                out_msg += std::to_string(elapsed) + " ms\n";
            } else if (m_time_unit == "s") {
                auto elapsed = std::chrono::duration<double>(time_diff).count();
                out_msg += std::to_string(elapsed) + " s\n";
            } else {
                auto elapsed = std::chrono::duration<double, std::micro>(time_diff).count();
                out_msg += std::to_string(elapsed) + " us\n";
            }

            std::cout << out_msg;
        }
        m_intevalMap[key] = cur_time;
    }

    void Hit(std::string msg, bool is_print = true,
             std::source_location loc = std::source_location::current()) {
        auto key = std::string(loc.file_name()) + std::string(loc.function_name()) +
                   std::to_string(loc.line()) + std::to_string(loc.column()) + msg;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_hit_count++;
        // 如果第一次添加进map, list
        if (m_hitMap.find(key) == m_hitMap.end()) {
            HitInfo hitinfo;
            hitinfo.msg = msg;
            hitinfo.time_vec.push_back(std::chrono::high_resolution_clock::now());
            hitinfo.count = 0;
            m_hitMap[key] = hitinfo;
            m_hitList.push_back(key);
        } else {
            // 修改信息
            auto& in_hitinfo = m_hitMap[key];
            in_hitinfo.time_vec.push_back(std::chrono::high_resolution_clock::now());
            in_hitinfo.count++;
        }

        if (m_hitList.size() < 2) {
            return;
        }  // 至少有两个hit才能计算时间差

        auto& cur_hitinfo = m_hitMap[key];
        int cur_index = 0, compare_index = 0;
        int need_count = 0;
        // 打印时间差，时间差是当前list自己的位置和前一个位置的时间差
        // 查看自己是否是第一个，如果是第一个，打印自己和最后一个的时间差
        for (auto& it : m_hitList) {
            if (it == key) {
                break;
            }
            cur_index++;
        }
        compare_index = cur_index - 1;
        if (compare_index < 0) {
            compare_index = m_hitList.size() - 1;
        }

        need_count = cur_hitinfo.count;
        // 根据当前的count，与之前对应count中存放的时间差
        auto& compare_hitinfo = m_hitMap[m_hitList[compare_index]];
        auto time_diff =
            cur_hitinfo.time_vec[need_count] - compare_hitinfo.time_vec[compare_hitinfo.count];

        std::string out_msg = "Hit: from " + compare_hitinfo.msg + " to " + cur_hitinfo.msg +
                              " in Count: " + std::to_string(need_count) + " Spend: ";

        if (m_time_unit == "ms") {
            auto elapsed = std::chrono::duration<double, std::milli>(time_diff).count();
            out_msg += std::to_string(elapsed) + " ms\n";
        } else if (m_time_unit == "s") {
            auto elapsed = std::chrono::duration<double>(time_diff).count();
            out_msg += std::to_string(elapsed) + " s\n";
        } else {
            auto elapsed = std::chrono::duration<double, std::micro>(time_diff).count();
            out_msg += std::to_string(elapsed) + " us\n";
        }

        if (is_print) {
            std::cout << out_msg;
        }

        if (m_is_write_file) {
            *m_file << out_msg;
            if (m_hit_count % 100 == 0) {
                m_file->flush();
            }
        }

        // 为了避免随着时间增加，map的时间戳越来越大，清理所有map的vec 一半最小
        // if(m_hit_count % 500 == 0 && m_hit_count > 0) {
        //     // 寻找最小的vec
        //     int min_size = INT_MAX;
        //     for(auto& it : m_hitMap) {
        //         if(it.second.time_vec.size() < min_size) {
        //             min_size = it.second.time_vec.size();
        //         }
        //     }

        //     min_size = min_size / 2;
        //     if(min_size <= 100) {
        //         return;
        //     }

        //     for(auto& it : m_hitMap) {
        //         it.second.time_vec.erase(it.second.time_vec.begin(), it.second.time_vec.begin() +
        //         min_size); it.second.count -= min_size;
        //     }
        // }
    }

    // "us", "ms", "s"
    void HitSetTimeUnit(std::string unit) { m_time_unit = unit; }

    void HitSetWriteFile(std::string file_name) {
        // 如果文件存在，清空文件
        m_file = new std::ofstream(file_name, std::ios::out | std::ios::trunc);
        m_is_write_file = true;
    }

    void HitClear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_hitMap.clear();
        m_hitList.clear();
    }

private:
    const char* getClassName() const override { return "bre::TimeTest"; }

    ~TimeTest() { delete m_file; }

    TimeTest() { m_is_write_file = false; }

    TimeTest(const TimeTest&) = delete;
    TimeTest& operator=(const TimeTest&) = delete;

private:
    // 针对Inteval的map， key是loc， value是时间
    std::map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> m_intevalMap;

    // 针对Hit的
    // hitmap_count， key是loc
    // hitlist，记录hit的顺序
    std::map<std::string, HitInfo> m_hitMap;
    std::vector<std::string> m_hitList;


    std::ofstream* m_file;
    bool m_is_write_file = false;

    // 输出时间差单位
    std::string m_time_unit = "ms";

    std::mutex m_mutex;

    uint64_t m_hit_count = 0;
};

}  // namespace bre
