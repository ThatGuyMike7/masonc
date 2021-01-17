#ifndef MASONC_TYPE_HPP
#define MASONC_TYPE_HPP

#include <common.hpp>

#include <string>

namespace masonc
{
    using type = const char*;
    using type_handle = u64;

    // Types defined and built into the programming language.
    inline const type TYPE_VOID = "void";

    inline const type TYPE_BOOL = "bool";
    inline const type TYPE_CHAR = "char";
    inline const type TYPE_U8 = "u8";
    inline const type TYPE_S8 = "s8";

    inline const type TYPE_U16 = "u16";
    inline const type TYPE_S16 = "s16";

    inline const type TYPE_U32 = "u32";
    inline const type TYPE_S32 = "s32";
    inline const type TYPE_F32 = "f32";

    inline const type TYPE_U64 = "u64";
    inline const type TYPE_S64 = "s64";
    inline const type TYPE_F64 = "f64";
}

#endif