#ifndef MASONC_VERSION_HPP
#define MASONC_VERSION_HPP

namespace masonc
{
    // Possibly incompatible changes, changes that might break existing code.
    #define MASONC_VERSION_MAJOR "0"

    // New features or changes to existing code that do not break existing code,
    // i.e. no build errors, but could contain semantic changes to existing implementations.
    #define MASONC_VERSION_MINOR "0"

    // Backwards compatible features or changes that neither break existing code,
    // nor semantically change existing implementations.
    #define MASONC_VERSION_PATCH "1"

    // Backwards compatible bug fixes only.
    #define MASONC_VERSION_REVISION "0"

    // Version string of both the language and the compiler.
    const char VERSION[] = MASONC_VERSION_MAJOR "."
                           MASONC_VERSION_MINOR "."
                           MASONC_VERSION_PATCH "."
                           MASONC_VERSION_REVISION;

    #undef MASONC_VERSION_MAJOR
    #undef MASONC_VERSION_MINOR
    #undef MASONC_VERSION_PATCH
    #undef MASONC_VERSION_REVISION
}

#endif