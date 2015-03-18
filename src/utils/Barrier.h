//
//  Barrier.h
//  SwiftSnails
//
//  Created by Chunwei on 3/17/15.
//  Copyright (c) 2015 Chunwei. All rights reserved.
//
#ifndef SwiftSnals_utils_Barrier_h_
#define SwiftSnals_utils_Barrier_h_
// a wrapper of condition_variable to block current thread
// and wait for future flag
namespace swift_snails {

class BaseBarrier {
public:
	virtual void block() = 0;
    virtual void unblock() = 0;
    virtual ~BaseBarrier() {}
};

class Barrier : public BaseBarrier {
public:
    explicit Barrier() {
    }
    virtual ~Barrier() {
    }
    // block current thread
    void block() {
        if(!is_block()) {
            std::lock_guard<std::mutex> lk(_flag_mut);
            _is_block = true;
        } 
        std::unique_lock<std::mutex> lk(_flag_mut);
        _block_cond.wait(lk, [this]{
                return ! is_block();
            });
    }
    // tell all the waiting thead to continue running
    void unblock() {
        // change block flag
        std::lock_guard<std::mutex> lk(_flag_mut);
        _is_block = false;
        _block_cond.notify_all();
    }

    bool is_block() const {
        return _is_block;
    }

private:
    // block the current thread
    mutable std::mutex _flag_mut;
    mutable std::mutex _block_mut;
    bool _is_block = false;
    std::condition_variable _block_cond;
};  // end class Barrier


// a more flexible Barrier
class CompBarrier : public BaseBarrier {

    typedef std::function<void()> void_lamb;

public:
    void block(void_lamb &set_flag, std::function<bool()> &cond_func) {
        set_flag();
        std::unique_lock<std::mutex> lk(_flag_mut);
        _block_cond.wait(lk, cond_func);
    }

    void unblock(void_lamb &func) {
        func(); // reset flag
        _block_cond.notify_all();
    }

    virtual void block() {
        throw std::bad_function_call();
    }
    virtual void unblock() {
        throw std::bad_function_call();
    }

    virtual ~CompBarrier() {
    }

private:
    mutable std::mutex _flag_mut;
    std::condition_variable _block_cond;
};  // end class FinishInitBarrier


};  // end namespace swift_snails
#endif