#ifndef PROXY_H
#define PROXY_H

template <typename T>
class ClassProxy {
    protected:
        T* ptr = nullptr;

    public:
        T* get() { return this->ptr; };

        virtual void initialize(void*) = 0;
};

#endif