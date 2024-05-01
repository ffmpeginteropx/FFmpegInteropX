template <typename T, auto ReleaseFunc>
struct AutoReleasePtr
{
    T* ptr = nullptr;

    AutoReleasePtr() = default;
    AutoReleasePtr(T* p) : ptr(p) { }

    T** operator&() { return &ptr; }
    T* operator->() { return ptr; }
    T& operator*() { return *ptr; }
    operator bool() const { return ptr; }

    ~AutoReleasePtr()
    {
        if (ptr)
        {
            if constexpr (std::is_same_v<decltype(ReleaseFunc), void(*)(T*)>)
                ReleaseFunc(ptr);
            else
                ReleaseFunc(&ptr);
        }
    }
};
