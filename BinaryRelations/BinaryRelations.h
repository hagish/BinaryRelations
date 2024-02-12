#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>

namespace BinaryRelations
{
// -------- Manipulate vector with unique sorted elements --------

template <typename T> int countInSortedVector(const std::vector<T> *vector, const T &value)
{
    typename std::vector<T>::const_iterator it = std::lower_bound(vector->cbegin(), vector->cend(), value);
    return it != vector->cend() && *it == value ? 1 : 0;
}

template <typename T>
typename std::vector<T>::const_iterator findInSortedVector(const std::vector<T> *vector, const T &value)
{
    typename std::vector<T>::const_iterator it = std::lower_bound(vector->cbegin(), vector->cend(), value);
    if (it != vector->cend() && *it == value)
        return it;
    else
        return vector->cend();
}

template <typename T> int insertIntoSortedVector(std::vector<T> *vector, const T &value)
{
    typename std::vector<T>::iterator it = std::lower_bound(vector->begin(), vector->end(), value);
    if (it != vector->end() && *it == value)
        return 0; // It's already there
    else
    {
        vector->insert(it, value);
        return 1;
    }
}

template <typename T> int removeFromSortedVector(std::vector<T> *vector, const T &value)
{
    typename std::vector<T>::iterator it = std::lower_bound(vector->begin(), vector->end(), value);
    if (it != vector->end() && *it == value)
    {
        vector->erase(it);
        return 1;
    }
    else
        return 0;
}

template <typename T>
void insertIntoSortedVector(std::vector<T> *sourceVector, std::vector<T> *insertVector, std::vector<T> *outVector)
{
    typename std::vector<T>::const_iterator sourceIt = sourceVector->cbegin();
    typename std::vector<T>::const_iterator insertIt = insertVector->cbegin();

    outVector->clear();
    outVector->reserve(sourceVector->count() + insertVector->count());

    while (sourceIt != sourceVector->cend() && insertIt != insertVector->cend())
    {
        if (*sourceIt == *insertIt)
        {
            outVector->append(*sourceIt++);
            insertIt++;
        }
        else if (*sourceIt < *insertIt)
            outVector->append(*sourceIt++);
        else
            outVector->append(*insertIt++);
    }

    while (sourceIt != sourceVector->cend())
        outVector->append(*sourceIt++);

    while (insertIt != insertVector->cend())
        outVector->append(*insertIt++);
}

template <typename T>
void removeFromSortedVector(std::vector<T> *sourceVector, std::vector<T> *removeVector, std::vector<T> *outVector)
{
    typename std::vector<T>::const_iterator sourceIt = sourceVector->cbegin();
    typename std::vector<T>::const_iterator removeIt = removeVector->cbegin();

    outVector->clear();
    outVector->reserve(std::max(sourceVector->count(), removeVector->count()));

    while (sourceIt != sourceVector->cend() && removeIt != removeVector->cend())
    {
        if (*sourceIt == *removeIt)
        {
            sourceIt++;
            removeIt++;
        }
        else if (*sourceIt < *removeIt)
            outVector->append(*sourceIt++);
        else
            outVector->append(*removeIt++);
    }

    while (sourceIt != sourceVector->cend())
        outVector->append(*sourceIt++);
}

// ----------------------------------------------------------------------------

/**
 @img html \image html binary-relations-one-to-many.png
 A one-to-many set of (left, right) pairs. The left side can have any number of right counterparts. The right side can only be in a pair with one left side.
 */
template <typename LeftType, typename RightType> class OneToMany
{
    std::unordered_map<LeftType, std::vector<RightType> *> m_LeftToRight;
    std::unordered_map<RightType, LeftType> m_RightToLeft;
    std::vector<RightType> m_EmptyRightVector;

public:
    /**
    @brief A pair of (left, right) values.
     */
    struct Pair
    {
        LeftType left;
        RightType right;
        Pair(LeftType left, RightType right) : left(left), right(right)
        {
        }
        Pair()
        {
        }
    };

    OneToMany()
    {
    }

    /**
     @brief Insert a pair into the set.
     The rule for one-on-many is that if the right value is part of an existing pair in the set, that relation will be removed.
     @param pair The (left, right) pair to insert.
     */
    void insert(const Pair &pair)
    {
        insert(pair.left, pair.right);
    }

    /**
     @brief Insert a (left, right) pair into the set.
     The rule for one-on-many is that if the right value is part of an existing pair in the set, that relation will be removed.
     @param left The left side of the pair to add.
     @param right The right side of the pair to add.
     */
    void insert(const LeftType &left, const RightType &right)
    {
        auto r2l_it = m_RightToLeft.find(right);
        if (r2l_it != m_RightToLeft.end())
        {
            if (r2l_it->second == left)
                return; // We already have this pair - do nothing

            remove(r2l_it->second, right); // Remove old relation
        }

        auto l2r_vec = m_LeftToRight[left]; // Will insert if it isn't already there.
        if (l2r_vec == nullptr)
        {
            l2r_vec = new std::vector<RightType>();
            m_LeftToRight[left] = l2r_vec;
        }
        insertIntoSortedVector(l2r_vec, right);

        m_RightToLeft[right] = left;
    }

    /**
     @brief Insert multiple  pairs into the set.
     The rule for one-on-many is that if the right value is part of an existing pair in the set, that relation will be removed.
     @param other The OneToMany to add.
     */
    void insert(const OneToMany<LeftType, RightType>& other)
    {
        for (auto pair : other)
        {
            insert(pair);
        }
    }

    /**
     @brief Remove a (left, right) pair from the set.
     If there is no matching pair in the set, nothing happens.
     @param pair The (left, right) pair to remove.
     */
    void remove(const Pair &pair)
    {
        remove(pair.left, pair.right);
    }

    /**
     @brief Remove a pair from the set.
     If there is no matching pair in the set, nothing happens.
     @param left The left side of the pair to remove.
     @param right The right side of the pair to remove.
     */
    void remove(const LeftType &left, const RightType &right)
    {
        auto r2l_it = m_RightToLeft.find(right);
        if (r2l_it == m_RightToLeft.end())
            return; // (*,right) not in the set - do nothing

        if (r2l_it->second != left)
            return; // (left,right) is not in the set - do nothing

        auto l2r_it = m_LeftToRight.find(left);
        auto l2r_vec = l2r_it->second;
        auto l2r_vec_it = findInSortedVector(l2r_vec, right);
        if (l2r_vec_it != l2r_vec->cend())
        {
            l2r_vec->erase(l2r_vec_it);
            if (l2r_vec->size() == 0)
            {
                m_LeftToRight.erase(l2r_it);
                delete l2r_vec; // Vector is empty now
            }
            m_RightToLeft.erase(r2l_it);
        }
    }

    /**
     @brief Remove all pairs with the given left value.
     If there is no matching pair in the set, nothing happens.
     @param left The left side of the pairs to remove.
     */
    void removeLeft(const LeftType &left)
    {
        auto l2r_it = m_LeftToRight.find(left);
        if (l2r_it == m_LeftToRight.end())
            return;

        auto l2r_vec = l2r_it->second;
        for (auto right : l2r_vec)
        {
            auto r2l_it = m_RightToLeft.find(right);
            m_RightToLeft.erase(r2l_it);
        }

        m_LeftToRight.erase(l2r_it);
        delete l2r_vec; // Vector is empty now
    }

    /**
     @brief Remove the pair with the given right value.
     If there is no matching pair in the set, nothing happens.
     @param right The right side of the pairs to remove.
     */
    void removeRight(const RightType &right)
    {
        auto r2l_it = m_RightToLeft.find(right);
        if (r2l_it == m_RightToLeft.end())
            return;
        remove(r2l_it->second, right);
    }

    /**
     @brief Remove multiple (left, right) pairs from the set.
     @param other The set of pairs to remove.
     */
    void remove(const OneToMany<LeftType, RightType> &other)
    {
        for (auto pair : other)
        {
            remove(pair);
        }
    }

    /**
     @brief Remove all pais=rs from the set.
     */
    void clear()
    {
        m_RightToLeft.clear();
        m_LeftToRight.clear();
    }

    bool contains(const Pair &pair) const
    {
        return contains(pair.left, pair.right);
    }

    bool contains(const LeftType &left, const RightType &right) const
    {
        auto r2l_it = m_RightToLeft.find(right);
        return r2l_it != m_RightToLeft.cend() && r2l_it->second == left;
    }

    bool containsLeft(const LeftType &left) const
    {
        return m_LeftToRight.contains(left);
    }

    bool containsRight(const RightType &right) const
    {
        return m_RightToLeft.contains(right);
    }

    const std::vector<RightType>* findRight(const LeftType &left) const
    {
        auto l2r_it = m_LeftToRight.find(left);
        if (l2r_it == m_LeftToRight.end())
            return &m_EmptyRightVector;

        return l2r_it->second;
    }

    LeftType findLeft(const RightType &right, const LeftType &notFoundValue) const
    {
        auto r2l_it = m_RightToLeft.find(right);
        if (r2l_it == m_RightToLeft.end())
            return notFoundValue;

        return r2l_it->second;
    }

    int countLeft() const
    {
        return (int)m_LeftToRight.size();
    }

    int countRight() const
    {
        return (int)m_RightToLeft.size();
    }

    int count() const
    {
        return (int)m_RightToLeft.size();
    }

    class Iterator
    {
      public:
        typename std::unordered_map<LeftType, std::vector<RightType> *>::const_iterator l2r_it;
        typename std::unordered_map<LeftType, std::vector<RightType> *>::const_iterator l2r_it_end;
        typename std::vector<RightType>::const_iterator l2r_vec_it;

        inline Pair operator*() const noexcept
        {
            auto left = l2r_it->first;
            auto right = *l2r_vec_it;
            return Pair(left, right);
        }

        inline bool operator==(const Iterator &other) const noexcept
        {
            return l2r_it == other.l2r_it;
        }

        inline bool operator!=(const Iterator &other) const noexcept
        {
            return l2r_it != other.l2r_it;
        }

        inline Iterator operator++() noexcept
        {
            l2r_vec_it++;
            if (l2r_vec_it == l2r_it->second->cend())
            {
                l2r_it++;
                if (l2r_it != l2r_it_end)
                {
                    l2r_vec_it = l2r_it->second->cbegin();
                }
            }
            return *this;
        }
    };

    Iterator begin() const
    {
        Iterator it;
        it.l2r_it = m_LeftToRight.cbegin();
        it.l2r_it_end = m_LeftToRight.cend();
        if (it.l2r_it != it.l2r_it_end)
        {
            auto l2r_vec = it.l2r_it->second;
            it.l2r_vec_it = l2r_vec->cbegin();
        }
        return it;
    }

    Iterator end() const
    {
        Iterator it;
        it.l2r_it = m_LeftToRight.cend();
        return it;
    }
};

// ----------------------------------------------------------------------------

template <typename LeftType, typename RightType> class ManyToMany
{
    std::unordered_map<LeftType, std::vector<RightType> *> m_LeftToRight;
    std::unordered_map<RightType, std::vector<LeftType> *> m_RightToLeft;
    std::vector<RightType> m_EmptyRightVector;
    std::vector<LeftType> m_EmptyLeftVector;
    int m_Count;

public:
    struct Pair
    {
        LeftType left;
        RightType right;
        Pair(LeftType left, RightType right) : left(left), right(right)
        {
        }
        Pair()
        {
        }
    };

    ManyToMany() : m_Count(0)
    {
    }

    ManyToMany(const ManyToMany &other) : m_Count(0), m_RightToLeft(other.m_RightToLeft), m_LeftToRight(other.m_LeftToRight)
    {
    }

    void insert(const Pair &pair)
    {
        insert(pair.left, pair.right);
    }

    void insert(const LeftType &left, const RightType &right)
    {
        auto l2r_it = m_LeftToRight.find(left);
        if (l2r_it != m_LeftToRight.end())
        {
            auto l2r_vec = l2r_it->second;
            if (0 != countInSortedVector(l2r_vec, right))
                return; // We already have this pair;
        }

        auto l2r_vec = m_LeftToRight[left]; // Will insert if it isn't already there.
        if (l2r_vec == nullptr)
        {
            l2r_vec = new std::vector<RightType>();
            m_LeftToRight[left] = l2r_vec;
        }
        insertIntoSortedVector(l2r_vec, right);

        auto r2l_vec = m_RightToLeft[right]; // Will insert if it isn't already there.
        if (r2l_vec == nullptr)
        {
            r2l_vec = new std::vector<LeftType>();
            m_RightToLeft[right] = r2l_vec;
        }
        insertIntoSortedVector(r2l_vec, left);
        m_Count += 1;
    }

    void remove(const Pair &pair)
    {
        remove(pair.left, pair.right);
    }

    void remove(const LeftType &left, const RightType &right)
    {
        auto l2r_it = m_LeftToRight.find(left);
        if (l2r_it != m_LeftToRight.end())
        {
            auto r2l_it = m_RightToLeft.find(right);
            if (r2l_it != m_RightToLeft.end())
            {
                auto l2r_vec = l2r_it->second;
                auto r2l_vec = r2l_it->second;

                removeFromSortedVector(l2r_vec, right);
                removeFromSortedVector(r2l_vec, left);

                if(l2r_vec->size() == 0)
                {
                    m_LeftToRight.erase(l2r_it);
                    delete l2r_vec;
                }

                if(r2l_vec->size() == 0)
                {
                    m_RightToLeft.erase(r2l_it);
                    delete r2l_vec;
                }
                
                m_Count -= 1;
            }
        }
    }

    void removeLeft(const LeftType &left)
    {
        auto l2r_it = m_LeftToRight.constFind(left);
        if (l2r_it != m_LeftToRight.end())
        {
            auto l2r_vec = l2r_it->second;
            for (auto right : l2r_vec)
            {
                auto r2l_it = m_RightToLeft.constFind(right);
                auto r2l_vec = r2l_it->second;
                removeFromSortedVector(*r2l_vec, left);
                m_Count -= 1;
                if (r2l_vec->size() == 0)
                {
                    m_RightToLeft.erase(r2l_it);
                    delete r2l_vec;
                }
            }
            m_LeftToRight.erase(l2r_it);
            delete l2r_vec;
        }
    }

    void removeRight(const RightType &right)
    {
        auto r2l_it = m_RightToLeft.constFind(right);
        if (r2l_it != m_RightToLeft.end())
        {
            auto r2l_vec = r2l_it->second;
            for (auto left : r2l_vec)
            {
                auto l2r_it = m_LeftToRight.constFind(left);
                auto l2r_vec = l2r_it->second;
                removeFromSortedVector(*l2r_vec, right);
                m_Count -= 1;
                if (l2r_vec->size() == 0)
                {
                    m_LeftToRight.erase(l2r_it);
                    delete l2r_vec;
                }
            }
            m_RightToLeft.erase(r2l_it);
            delete r2l_vec;
        }
    }

    void insert(const ManyToMany<LeftType, RightType>& other)
    {
        for (auto pair : other)
        {
            insert(pair);
        }
    }

    void remove(const ManyToMany<LeftType, RightType>& other)
    {
        for (auto pair : other)
        {
            remove(pair);
        }
    }

    void clear()
    {
        m_RightToLeft.clear();
        m_LeftToRight.clear();
    }

    bool contains(const Pair &pair) const
    {
        return contains(pair.left, pair.right);
    }

    bool contains(const LeftType &left, const RightType &right) const
    {
        auto l2r_it = m_LeftToRight.find(left);
        if (l2r_it != m_LeftToRight.end())
        {
            auto r2l_it = m_RightToLeft.find(right);
            if (r2l_it != m_RightToLeft.end())
            {
                auto l2r_vec = l2r_it->second;
                auto r2l_vec = r2l_it->second;
                if (l2r_vec->size() < r2l_vec->size())
                {
                    return 0 != countInSortedVector(l2r_vec, right);
                }
                else
                {
                    return 0 != countInSortedVector(r2l_vec, left);
                }
            }
        }
        return false;
    }

    bool containsLeft(const LeftType &left) const
    {
        return m_LeftToRight.contains(left);
    }

    bool containsRight(const RightType &right) const
    {
        return m_RightToLeft.contains(right);
    }

    const std::vector<RightType>* findRight(const LeftType &left) const
    {
        auto l2r_it = m_LeftToRight.find(left);
        if (l2r_it == m_LeftToRight.end())
            return &m_EmptyRightVector;

        return l2r_it->second;
    }

    const std::vector<LeftType>* findLeft(const RightType &right) const
    {
        auto r2l_it = m_RightToLeft.find(right);
        if (r2l_it == m_RightToLeft.end())
            return &m_EmptyLeftVector;

        return r2l_it->second;
    }

    int countLeft() const
    {
        return m_LeftToRight.count();
    }

    int countRight() const
    {
        return m_RightToLeft.count();
    }

    int count() const
    {
        return m_Count;
    }

    class Iterator
    {
      public:
        typename std::unordered_map<LeftType, std::vector<RightType> *>::const_iterator l2r_it;
        typename std::unordered_map<LeftType, std::vector<RightType> *>::const_iterator l2r_it_end;
        typename std::vector<RightType>::const_iterator l2r_vec_it;

        inline Pair operator*() const noexcept
        {
            auto left = l2r_it->first;
            auto right = *l2r_vec_it;
            return Pair(left, right);
        }

        inline bool operator==(const Iterator &other) const noexcept
        {
            return l2r_it == other.LeftIterator;
        }

        inline bool operator!=(const Iterator &other) const noexcept
        {
            return l2r_it != other.l2r_it;
        }

        inline Iterator operator++() noexcept
        {
            l2r_vec_it++;
            if (l2r_vec_it == (*l2r_it).second->cend())
            {
                l2r_it++;
                if (l2r_it != l2r_it_end)
                {
                    l2r_vec_it = (*l2r_it).second->cbegin();
                }
            }
            return *this;
        }
    };

    Iterator begin() const
    {
        Iterator it;
        it.l2r_it = m_LeftToRight.cbegin();
        it.l2r_it_end = m_LeftToRight.cend();
        if (it.l2r_it != it.l2r_it_end)
        {
            auto l2r_vec = it.l2r_it->second;
            it.l2r_vec_it = l2r_vec->cbegin();
        }
        return it;
    }

    Iterator end() const
    {
        Iterator it;
        it.l2r_it = m_LeftToRight.cend();
        return it;
    }
};

// ----------------------------------------------------------------------------

template <typename LeftType, typename RightType> class OneToOne
{
    std::unordered_map<LeftType, RightType> m_LeftToRight;
    std::unordered_map<RightType, LeftType> m_RightToLeft;

public:
    struct Pair
    {
        LeftType left;
        RightType right;
        Pair(LeftType left, RightType right) : left(left), right(right)
        {
        }
        Pair()
        {
        }
    };

    OneToOne()
    {
    }

    OneToOne(const OneToOne &other) : m_RightToLeft(other.m_RightToLeft), m_LeftToRight(other.m_LeftToRight)
    {
    }

    void insert(const Pair &pair)
    {
        insert(pair.left, pair.right);
    }

    void insert(const LeftType &left, const RightType &right)
    {
        if (!contains(left, right))
        {
            removeLeft(left);
            removeRight(right);
            m_RightToLeft[right] = left;
            m_LeftToRight[left] = right;
        }
    }

    void remove(const Pair &pair)
    {
        remove(pair.left, pair.right);
    }

    void remove(const LeftType &left, const RightType &right)
    {
        if (contains(left, right))
        {
            auto l2r_it = m_LeftToRight.find(left);
            auto r2l_it = m_RightToLeft.find(right);

            m_LeftToRight.erase(l2r_it);
            m_RightToLeft.erase(r2l_it);
        }
    }

    void removeLeft(const LeftType &left)
    {
        auto l2r_it = m_LeftToRight.find(left);
        if (l2r_it != m_LeftToRight.end())
        {
            auto right = l2r_it->second;
            auto r2l_it = m_RightToLeft.find(right);
            m_RightToLeft.erase(r2l_it);
            m_LeftToRight.erase(l2r_it);
        }
    }

    void removeRight(const RightType &right)
    {
        auto r2l_it = m_RightToLeft.find(right);
        if (r2l_it != m_RightToLeft.end())
        {
            auto left = r2l_it->second;
            auto l2r_it = m_LeftToRight.find(left);
            m_LeftToRight.erase(l2r_it);
            m_RightToLeft.erase(r2l_it);
        }
    }

    void insert(const OneToOne<LeftType, RightType> &other)
    {
        for (auto pair : other)
        {
            insert(pair);
        }
    }

    void remove(const OneToOne<LeftType, RightType> &other)
    {
        for (auto pair : other)
        {
            remove( pair);
        }
    }

    void clear()
    {
        m_RightToLeft.clear();
        m_LeftToRight.clear();
    }

    bool contains(const Pair &pair) const
    {
        return contains(pair.left, pair.right);
    }

    bool contains(const LeftType &left, const RightType &right) const
    {
        auto l2r_it = m_LeftToRight.find(left);
        return l2r_it != m_LeftToRight.end() && l2r_it->second == right;
    }

    bool containsLeft(const LeftType &left) const
    {
        auto l2r_it = m_LeftToRight.find(left);
        return l2r_it != m_LeftToRight.end();
    }

    bool containsRight(const RightType &right) const
    {
        auto r2l_it = m_RightToLeft.find(right);
        return r2l_it != m_RightToLeft.end();
    }

    RightType findRight(const LeftType &left, const RightType &defaultValue) const
    {
        auto l2r_it = m_LeftToRight.find(left);
        return l2r_it != m_LeftToRight.end() ? l2r_it->second : defaultValue;
    }

    LeftType findLeft(const RightType &right, const LeftType &defaultValue) const
    {
        auto r2l_it = m_RightToLeft.find(right);
        return r2l_it != m_RightToLeft.end() ? r2l_it->second : defaultValue;
    }

    int countLeft() const
    {
        return (int)m_LeftToRight.size();
    }

    int countRight() const
    {
        return (int)m_RightToLeft.size();
    }

    int count() const
    {
        return (int)m_LeftToRight.size();
    }

    class Iterator
    {
      public:
        std::unordered_map<LeftType, RightType>::const_iterator l2r_it;

        inline Pair operator*() const noexcept
        {
            return Pair(l2r_it->first, l2r_it->second);
        }

        inline bool operator==(const Iterator &other) const noexcept
        {
            return l2r_it == other.l2r_it;
        }
        inline bool operator!=(const Iterator &other) const noexcept
        {
            return l2r_it != other.l2r_it;
        }
        inline Iterator operator++() noexcept
        {
            l2r_it++;
            return *this;
        }
    };

    Iterator begin() const
    {
        Iterator it;
        it.l2r_it = m_LeftToRight.cbegin();
        return it;
    }

    Iterator end() const
    {
        Iterator it;
        it.l2r_it = m_LeftToRight.cend();
        return it;
    }
};
// ----------------------------------------------------------------------------
} // namespace BinaryRelations