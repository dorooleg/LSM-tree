#pragma once

#include <string>
#include <cstdint>
#include <experimental/filesystem>
#include <algorithm>
#include <vector>
#include <optional>
#include <ostream>
#include <thread>
#include "FSVector.h"

class LSMTree final
{
public:
    using key = uint64_t;

    explicit LSMTree(const std::string& directory)
        : directory_(directory)
    {
        cache_.reserve(CACHE_SIZE);
        std::experimental::filesystem::create_directories(directory);
    }

    void flush()
    {
        merge(cache_, 0);
        cache_.clear();
        for (uint64_t level = 0; is_exists_level(level); level++) {
            FSVector<key> from(directory_ + "/level" + std::to_string(level));
            if (from.size() > 10 * CACHE_SIZE * (level + 1)) {
                FSVector<key> to(directory_ + "/level" + std::to_string(level + 1));
                merge(from, to);
                from.clear();
            }
        }
    }

    void insert(const key& value)
    {
        if (cache_.size() == CACHE_SIZE) {
            flush();
        }

        if (auto it = std::lower_bound(cache_.begin(), cache_.end(), value); it == cache_.end() || *it != value) {
            cache_.insert(it, value);
        }
    }

    void remove(const key& value)
    {
        auto it = lower_bound(cache_.begin(), cache_.end(), value);
        if (it != cache_.end() && *it == value) {
            cache_.erase(it);
        }

        for (uint64_t level = 0; is_exists_level(level); level++) {
            FSVector<key> v(get_level_path(level));
            if (uint64_t idx = find_disk(value, level); idx < v.size()) {
                v.erase(idx);
            }
        }
    }

    bool find(const key& value) const
    {
        if (find_cache(value)) {
            return true;
        }

        for (uint64_t level = 0; is_exists_level(level); level++) {
            FSVector<key> v(get_level_path(level));
            if (find_disk(value, level) < v.size()) {
                return true;
            }
        }

        return false;
    }

    friend std::ostream& operator<<(std::ostream &strm, const LSMTree& tree)
    {
        strm << "Cache: " << tree.cache_.size();
        if (!tree.cache_.empty()) {
            strm <<std::endl;
        }

        for (const auto& value : tree.cache_) {
            strm << value << " ";
        }

        for (uint64_t level = 0; tree.is_exists_level(level); level++) {
            FSVector<key> v(tree.get_level_path(level));
            strm << std::endl << "Level" << level << ": " << v.size();
            if (!v.empty()) {
                strm << std::endl;
            }
            for (uint64_t i = 0; i < v.size(); i++) {
                strm << v[i] << " ";
            }
        }

        return strm;
    }

    ~LSMTree()
    {
        flush();
    }

private:
    template<typename F, typename T>
    void merge(const F& from, T& to) const
    {
        int64_t l = from.size() - 1;
        int64_t r = to.size() - 1;
        int64_t end = l + r + 1 - count_duplicates(from, to);
        to.resize(end + 1);
        while (l >= 0) {
            if (r >= 0 && from[l] <= to[r]) {
                if (from[l] == to[r]) {
                    l--;
                }
                to[end] = to[r];
                r--;
            } else {
                to[end] = from[l];
                l--;
            }
            end--;
        }
    }

    template<typename F, typename T>
    int64_t count_duplicates(const F& from, T& to) const
    {
        int64_t count = 0;
        for (uint64_t l = 0, r = 0; l < from.size() && r < to.size();) {
            auto f = from[l];
            auto t = to[r];
            if (f == t) {
                ++count;
                l++;
                r++;
            } else if (f < t) {
                l++;
            } else {
                r++;
            }
        }

        return count;
    }

    void merge(const std::vector<key>& from, uint64_t level_to) const
    {
        FSVector<key> to(get_level_path(level_to));
        merge(from, to);
    }

    bool find_cache(key value) const
    {
        return std::binary_search(cache_.begin(), cache_.end(), value);
    }

    std::string get_level_path(uint64_t level) const
    {
        return directory_ + "/level" + std::to_string(level);
    }

    bool is_exists_level(uint64_t level) const
    {
        return std::experimental::filesystem::exists(get_level_path(level));
    }

    uint64_t find_disk(key value, uint64_t level) const
    {
        FSVector<key> v(get_level_path(level));
        int64_t l = 0;
        int64_t r = v.size() - 1;
        while (l <= r) {
            int64_t m = l + (r - l) / 2;
            key data = v[m];

            if (data == value) {
                return static_cast<uint64_t>(m);
            }

            (data < value ? l : r) = data < value ? m + 1 : m - 1;
        }

        return v.size();
    }

private:
    std::string directory_;
    std::vector<key> cache_;
    const static uint64_t MEMORY_SIZE = 2 * 1024 * 1024; // 2 Mib
    const static uint64_t CACHE_SIZE = MEMORY_SIZE / sizeof(key);
    static_assert(MEMORY_SIZE % sizeof(key) == 0, "The key is not a multiple of the cache size. The location is not efficient");
    static_assert(MEMORY_SIZE > sizeof(key), "The key is larger than the cache size");
};
