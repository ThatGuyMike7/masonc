#ifndef MASONC_BUILD_HPP
#define MASONC_BUILD_HPP

#include <common.hpp>
#include <io.hpp>
#include <parser.hpp>

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

namespace masonc
{
    // Highest level object that allows building object files, executables, and so on.
    struct builder
    {
        builder(std::vector<path> sources,
                // Threads to use, must be either 0 or >= 2.
                // If the value is 0, max(2, "std::thread::hardware_concurrency()") is assumed.
                // If the value is 1, 2 is assumed.
                u64 overwrite_thread_count = 0,
                // How many bytes to read at minimum before synchronizing.
                // If a line contains 40 characters on average, this will synchronize
                // after reading at least 6553 lines.
                u64 min_bytes_for_sync = 1024 * 256);

    private:
        void do_work(u64 thread_index);

        // Split remaining work in "file_queue" between threads as evenly as possible,
        // increasing "file_queue_first" until the current queue is split and delegated to workers.
        void split_work();

        // Returns a list of file paths from a list of "path".
        //
        // A "path" can be either a file, directory, or recursive directory,
        // and we use this function to get actual file paths from those.
        std::vector<std::string> concrete_file_paths(const std::vector<path>& sources) const;

        u64 worker_thread_count;

        // Protects "all_work", "file_queue", "file_sizes", "file_queue_first", and "no_more_work".
        std::shared_mutex file_queue_mutex;
        std::condition_variable_any file_queue_condition;

        // Each thread has an index to an element here,
        // and each element here says which elements of "file_queue" are work for the given thread.
        std::vector<std::vector<u64>> all_work;

        // TODO: Free at some point.
        // FIXME: Maybe switch to "cstring_collection"?
        std::vector<const char*> file_queue;
        std::vector<u64> file_sizes;
        u64 file_queue_first = 0;

        // Quit condition for worker threads.
        bool no_more_work = false;

        // Protects "parse_outputs".
        std::mutex parse_output_mutex;
        std::vector<masonc::parser::parser_instance_output> parse_output;
    };
}

#endif