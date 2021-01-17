#ifndef MASONC_LANGUAGE_HPP
#define MASONC_LANGUAGE_HPP

#include <type.hpp>
#include <symbol.hpp>

namespace masonc
{
    // Master initialize function, call before doing anything else.
    void initialize_language();

    struct value_t;
    struct procedure_t
    {
        type return_type;
        value_t** arguments;
    };

    // Pointer to a callable procedure.
    struct procedure_pointer_t
    {
        procedure_t* procedure;
    };

    struct variable_t
    {
        type type;
    };

    // Pointer to a value type.
    struct pointer_t
    {
        value_t* value;
    };

    // Value type, something that is assignable.
    struct value_t
    {
        enum
        {
            VARIABLE,
            POINTER,
            PROCEDURE_POINTER
        } type;

        union
        {
            variable_t variable;
            pointer_t pointer;
            procedure_pointer_t procedure_pointer;
        };
    };

    struct struct_t
    {
        symbol name;
    };

    struct union_t
    {
        symbol name;
    };

    struct data_type_t
    {
        enum
        {
            STRUCT,
            UNION
        } type;

        union
        {
            struct_t struct_type;
            union_t union_type;
        };
    };

    struct object
    {
        enum
        {
            NONE,
            PROCEDURE,
            VARIABLE,
            POINTER,
            PROCEDURE_POINTER,
            STRUCT_TYPE,
            UNION_TYPE
        } type = NONE;

        union
        {
            procedure_t procedure;

            variable_t variable;
            pointer_t pointer;
            procedure_pointer_t procedure_pointer;

            struct_t struct_type;
            union_t union_type;
        };
    };
}

#endif