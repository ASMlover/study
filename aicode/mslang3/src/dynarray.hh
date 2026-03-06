#pragma once

#include "common.hh"

#include <utility>
#include <cstring>

namespace ms {

/**
 * @brief 动态数组模板类
 * 
 * 用于字节码块的常量池等场景，支持动态扩容，
 * 使用1.5倍增长策略平衡内存使用和扩容频率。
 * 
 * @tparam T 元素类型
 */
template <typename T>
class DynArray {
public:
    /**
     * @brief 默认构造函数，创建空数组
     */
    DynArray() = default;

    /**
     * @brief 析构函数，释放内存
     */
    ~DynArray() {
        delete[] data_;
    }

    /**
     * @brief 移动构造函数
     */
    DynArray(DynArray&& other) noexcept
        : data_(other.data_)
        , count_(other.count_)
        , capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.count_ = 0;
        other.capacity_ = 0;
    }

    /**
     * @brief 移动赋值运算符
     */
    DynArray& operator=(DynArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            count_ = other.count_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.count_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // 禁止拷贝
    DynArray(const DynArray&) = delete;
    DynArray& operator=(const DynArray&) = delete;

    /**
     * @brief 在数组末尾添加元素
     * @param value 要添加的元素
     */
    void write(T value) {
        if (count_ >= capacity_) {
            grow();
        }
        data_[count_++] = std::move(value);
    }

    /**
     * @brief 清空数组（不释放内存）
     */
    void clear() {
        count_ = 0;
    }

    /**
     * @brief 索引访问（非const）
     * @param index 索引位置
     * @return 元素的引用
     */
    T& operator[](usize index) {
        MS_ASSERT(index < count_, "Index out of bounds");
        return data_[index];
    }

    /**
     * @brief 索引访问（const）
     * @param index 索引位置
     * @return 元素的const引用
     */
    const T& operator[](usize index) const {
        MS_ASSERT(index < count_, "Index out of bounds");
        return data_[index];
    }

    /**
     * @brief 获取当前元素数量
     * @return 元素数量
     */
    [[nodiscard]] usize count() const { return count_; }

    /**
     * @brief 获取当前容量
     * @return 容量大小
     */
    [[nodiscard]] usize capacity() const { return capacity_; }

    /**
     * @brief 检查数组是否为空
     * @return 如果为空返回true
     */
    [[nodiscard]] bool isEmpty() const { return count_ == 0; }

    // 迭代器支持
    [[nodiscard]] T* begin() { return data_; }
    [[nodiscard]] T* end() { return data_ + count_; }
    [[nodiscard]] const T* begin() const { return data_; }
    [[nodiscard]] const T* end() const { return data_ + count_; }

private:
    T* data_ = nullptr;        ///< 数据指针
    usize count_ = 0;          ///< 当前元素数量
    usize capacity_ = 0;       ///< 当前容量

    /**
     * @brief 扩容数组，使用1.5倍增长策略
     */
    void grow() {
        // 1.5倍增长策略，首次扩容时至少为8
        usize newCapacity = capacity_ < 8 ? 8 : capacity_ + (capacity_ >> 1);
        
        T* newData = new T[newCapacity];
        
        // 移动已有元素
        for (usize i = 0; i < count_; i++) {
            newData[i] = std::move(data_[i]);
        }
        
        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }
};

} // namespace ms
