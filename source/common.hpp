#ifndef $_MASONC_COMMON_HPP_$
#define $_MASONC_COMMON_HPP_$

#include <iostream>
#include <cstdint>
#include <cstdlib>

namespace masonc
{
	using s8 = int8_t;
	using s16 = int16_t;
	using s32 = int32_t;
	using s64 = int64_t;

	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;
	
	using f32 = float;
	using f64 = double;
	
	//using rune = s32;
	
	void assume(bool statement, const char* msg = "", int code = -1);

    /*
    template <typename T>
    class result
    {
    public:
        result() : is_ok(false) { }
        result(T value) : _value(value), is_ok(true) { }
        ~result() {
            if (is_ok)
                _value.~T();
        }

        explicit operator bool() const { return is_ok; }
        
        T value() {
            assume(is_ok, "\"is_ok\" in std::optional<T> is false");
            return _value;
        }

    private:
        union { T _value; };
        bool is_ok;
    };

	template <typename T>
	class _result
	{
	public:
		_result() : is_ok(false) { }
		_result(T _value) : is_ok(true), _value(_value) { }
			
		//operator bool&() { return is_ok; }
		explicit operator bool() const { return is_ok; }
			
		T value()
		{
			assume(is_ok, "\"is_ok\" in std::optional<T> is false");
			return _value;
		}
		
	private:
		T _value;
		bool is_ok;
	};
    */
}

#endif