#ifndef __CPP_11_BACKPORT_H__
#define __CPP_11_BACKPORT_H__

#include <type_traits>

#if __cplusplus < 201703L

namespace std 
{
    
}

#endif

#if __cplusplus < 201402L

namespace std 
{
    
    template <class _Ty>
    using make_unsigned_t = typename make_unsigned<_Ty>::type;

#ifdef CPP11_BACKPORT_ENABLE_MAKE_UNIQUE
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
#endif 

} //namespace std 


#endif

#endif //__CPP_11_BACKPORT_H__
