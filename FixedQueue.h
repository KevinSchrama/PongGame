//
// Created by kevin on 30/05/2024.
//

#ifndef PONGGAME_FIXEDQUEUE_H
#define PONGGAME_FIXEDQUEUE_H

#include <queue>
#include <deque>

template <typename T, int MaxLen, typename Container=std::deque<T>>

class FixedQueue : public std::queue<T, Container> {
public:
    void push(const T& value) {
        if (this->size() == MaxLen) {
           this->c.pop_front();
        }
        std::queue<T, Container>::push(value);
    }

    bool check_space(){
        return this->size() < MaxLen;
    }
};

#endif //PONGGAME_FIXEDQUEUE_H
