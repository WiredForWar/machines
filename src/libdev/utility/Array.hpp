#pragma once

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <optional>

namespace Utility
{

template <class T, int StackCapacity> class Array
{
public:
    constexpr Array() = default;
    constexpr Array(std::initializer_list<T> list);
    Array(T (&Array)[StackCapacity]);

    const T& at(std::size_t Index) const { return m_Data[Index]; }
    T operator[](std::size_t Index) const { return m_Data[Index]; }
    T& operator[](std::size_t Index) { return m_Data[Index]; }

    T* data() { return &m_Data[0]; }
    const T* data() const { return &m_Data[0]; }

    T& first() { return m_Data[0]; }
    T& last() { return m_Data[m_Size - 1]; }

    const T& first() const { return m_Data[0]; }
    const T& last() const { return m_Data[m_Size - 1]; }

    void erase(const T* pItem);

    constexpr std::size_t size() const;

    void resize(std::size_t NewSize);

    constexpr std::size_t capacity() const;

    bool empty() const { return isEmpty(); }
    bool isEmpty() const;

    std::optional<std::size_t> indexOf(const T& Item) const;

    constexpr bool contains(const T& Item) const;

    constexpr void add(const T& Value);

    void removeLast();

    bool removeOne(const T& Item);

    void removeAt(std::size_t Index);

    void clear();

    T* begin() { return &m_Data[0]; }
    T* end() { return &m_Data[m_Size]; }

    auto rbegin() { return std::reverse_iterator(end()); }
    auto rend() { return std::reverse_iterator(begin()); }

    const T* begin() const { return &m_Data[0]; }
    const T* end() const { return &m_Data[m_Size]; }

    using size_type = std::size_t;

protected:
    T m_Data[StackCapacity] = {};
    std::size_t m_Size = 0;
};

template <class T, int StackCapacity> inline constexpr Array<T, StackCapacity>::Array(std::initializer_list<T> list)
{
    for (const T& Element : list)
    {
        add(Element);
    }
}

template <class T, int StackCapacity> inline Array<T, StackCapacity>::Array(T (&Array)[StackCapacity])
{
    for (const T& Element : Array)
    {
        add(Element);
    }
}

template <class T, int StackCapacity> void Array<T, StackCapacity>::erase(const T* pItem)
{
    std::ptrdiff_t Offset = pItem - begin();
    removeAt(Offset);
}

template <class T, int StackCapacity> constexpr std::size_t Array<T, StackCapacity>::size() const
{
    return m_Size;
}

template <class T, int StackCapacity> void Array<T, StackCapacity>::resize(std::size_t NewSize)
{
    m_Size = NewSize;
}

template <class T, int StackCapacity> constexpr std::size_t Array<T, StackCapacity>::capacity() const
{
    return StackCapacity;
}

template <class T, int StackCapacity> bool Array<T, StackCapacity>::isEmpty() const
{
    return m_Size == 0;
}

template <class T, int StackCapacity> std::optional<std::size_t> Array<T, StackCapacity>::indexOf(const T& Item) const
{
    for (std::size_t i = 0; i < size(); ++i)
    {
        if (m_Data[i] == Item)
            return i;
    }
    return {};
}

template <class T, int StackCapacity> inline constexpr bool Array<T, StackCapacity>::contains(const T& Item) const
{
    return indexOf(Item) >= 0;
}

template <class T, int StackCapacity> inline constexpr void Array<T, StackCapacity>::add(const T& Value)
{
    m_Data[m_Size] = Value;
    ++m_Size;
}

template <class T, int StackCapacity> void Array<T, StackCapacity>::removeLast()
{
    --m_Size;
}

template <class T, int StackCapacity> bool Array<T, StackCapacity>::removeOne(const T& Item)
{
    auto OptIndex = indexOf(Item);
    if (!OptIndex.has_value())
        return false;

    removeAt(OptIndex.value());
    return true;
}

template <class T, int StackCapacity> void Array<T, StackCapacity>::removeAt(std::size_t Index)
{
    for (std::size_t i = Index; i < m_Size - 1; ++i)
    {
        m_Data[i] = m_Data[i + 1];
    }
    --m_Size;
}

template <class T, int StackCapacity> void Array<T, StackCapacity>::clear()
{
    m_Size = 0;
}

} // namespace Utility

namespace std
{

template <class T, int StackCapacity, typename Predicate>
inline typename Utility::Array<T, StackCapacity>::size_type
erase_if(Utility::Array<T, StackCapacity>& container, Predicate predicate)
{
    const auto HadSize = container.size();

    for (auto it = container.rbegin(); it != container.rend(); ++it)
    {
        if (predicate(*it))
            container.erase(&*it);
    }

    return HadSize - container.size();
}

} // namespace std
