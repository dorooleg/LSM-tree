#pragma once

#include <fstream>
#include <memory>
#include <cstddef>
#include <string>
#include <experimental/filesystem>

template<typename T>
class FSVector final
{
public:
    using size_type = uint64_t;
    using value_type = T;

    class Reference
    {
        friend FSVector;
        Reference(const std::string& path, size_type n)
            : file_(path)
            , n_(n)
        {
        }

    public:
        Reference& operator=(const Reference& value)
        {
            return operator=((T)value);
        }

        Reference& operator=(const T& value)
        {
            file_.seekp(sizeof(size_type) + n_ * sizeof(T));
            file_.write(reinterpret_cast<const char*>(&value), sizeof(T));
            return *this;
        }

        operator T() const
        {
            std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(sizeof(T));
            file_.seekg(sizeof(size_type) + n_ * sizeof(T));
            file_.read(reinterpret_cast<char*>(data.get()), sizeof(T));
            return *reinterpret_cast<T*>(data.get());
        }

    private:
        mutable std::fstream file_;
        size_type n_;
    };

    explicit FSVector(const std::string& path)
        : path_(path)
    {
        if (!std::experimental::filesystem::exists(path)) {
            size_type size = 0;
            std::ofstream ofs(path, std::ifstream::out | std::ifstream::app | std::ifstream::binary);
            ofs.write(reinterpret_cast<char *>(&size), sizeof(size));
        }

        file_.open(path, std::ifstream::out | std::ifstream::in | std::ifstream::binary);
    }

    T operator[](size_type n) const
    {
        std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(sizeof(T));
        file_.seekg(sizeof(size_type) + n * sizeof(T));
        file_.read(reinterpret_cast<char*>(data.get()), sizeof(T));
        return *reinterpret_cast<T*>(data.get());
    }

    Reference operator[](size_type n)
    {
        file_.flush();
        return Reference(path_, n);
    }

    void push_back(const T& value)
    {
        size_type size = this->size();
        file_.seekp(sizeof(size_type) + size * sizeof(T));
        file_.write(reinterpret_cast<const char*>(&value), sizeof(T));
        ++size;
        file_.seekp(0);
        file_.write(reinterpret_cast<char*>(&size), sizeof(size));
    }

    size_type size() const
    {
        size_type size = 0;
        file_.seekg(0);
        file_.read(reinterpret_cast<char*>(&size), sizeof(size));
        return size;
    }

    void resize(size_type n)
    {
        file_.seekp(0);
        file_.write(reinterpret_cast<char*>(&n), sizeof(n));
    }

    void clear()
    {
        resize(0);
    }

    bool empty() const
    {
        return size() == 0;
    }

    void erase(size_type n)
    {
        const size_type size = this->size();
        for (size_type i = n; i + 1 < size; i++) {
            operator[](i) = operator[](i + 1);
        }
        resize(size - 1);
    }

private:
    mutable std::fstream file_;
    std::string path_;
};
