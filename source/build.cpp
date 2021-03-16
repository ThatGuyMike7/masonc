#include <build.hpp>

#include <common.hpp>
#include <io.hpp>
#include <logger.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <llvm_converter.hpp>
#include <language.hpp>

namespace masonc
{
    builder::builder(std::vector<path> sources, u64 overwrite_thread_count,
                     u64 min_bytes_for_sync)
    {
        if (overwrite_thread_count == 0) {
            worker_thread_count = static_cast<u64>(std::thread::hardware_concurrency());
        }
        else {
            worker_thread_count = overwrite_thread_count;
        }

        // TODO: Mark as unlikely.
        if (worker_thread_count < 2) {
            worker_thread_count = 1;
        }

        std::vector<std::thread> threads;
        threads.reserve(worker_thread_count);
        all_work.reserve(worker_thread_count);

        for (u64 i = 0; i < worker_thread_count; i += 1) {
            all_work.emplace_back();
            // FIXME: Protect "this" with a mutex maybe?
            threads.emplace_back(std::thread{ &builder::do_work, this, i });
        }

        std::vector<std::string> file_paths = concrete_file_paths(sources);

        // To be synced with member vectors.
        std::vector<char*> files;
        std::vector<u64> sizes;

        parse_output.reserve(file_paths.size());
        file_queue.reserve(file_paths.size());
        file_sizes.reserve(file_paths.size());
        files.reserve(file_paths.size());
        sizes.reserve(file_paths.size());

        // Bytes read since last sync.
        u64 bytes_read = 0;

        for (u64 i = 0; i < file_paths.size(); i += 1) {
            u64 contents_size;
            char* contents = file_read(file_paths[i].c_str(), 64000, &contents_size);

            // TODO: Mark as unlikely.
            if (contents == nullptr) {
                // TODO: Error.
            }
            else {
                bytes_read += contents_size;

                files.push_back(contents);
                sizes.push_back(contents_size);

                // Time to sync?
                if (bytes_read > min_bytes_for_sync) {
                    std::unique_lock<std::shared_mutex> file_queue_unique_lock{ file_queue_mutex };

                    file_queue.insert(file_queue.end(), files.begin(), files.end());
                    file_sizes.insert(file_sizes.end(), sizes.begin(), sizes.end());

                    split_work();

                    file_queue_unique_lock.unlock();
                    file_queue_condition.notify_all();

                    files.clear();
                    sizes.clear();
                    bytes_read = 0;
                }
            }
        }

        std::unique_lock<std::shared_mutex> file_queue_unique_lock{ file_queue_mutex };

        // Perhaps we read some last files without reaching "min_bytes_for_sync".
        // Sync the rest if we have anything.
        if (files.size() > 0) {

            file_queue.insert(file_queue.end(), files.begin(), files.end());
            file_sizes.insert(file_sizes.end(), sizes.begin(), sizes.end());

            split_work();
        }

        no_more_work = true;

        file_queue_unique_lock.unlock();
        file_queue_condition.notify_all();

        for (u64 i = 0; i < threads.size(); i += 1) {
            threads[i].join();
        }
    }

    void builder::do_work(u64 thread_index)
    {
        std::shared_lock<std::shared_mutex> file_queue_shared_lock{ file_queue_mutex };

        const std::vector<u64>& work = all_work[thread_index];
        u64 work_index = 0;

        // To be synced with "parse_outputs".
        std::vector<masonc::parser::parser_instance_output> buffered_parse_output;
        std::vector<masonc::parser::parser_instance> parsers;

        masonc::lexer::lexer_instance lexer;

        while (!no_more_work)
        {
            u64 i = work_index;

            LOOP:
            if (i < work.size())
            {
                // Do the work.
                {
                    auto* current_parse_output = &buffered_parse_output.emplace_back();

                    lexer.tokenize(file_queue[work[i]], file_sizes[work[i]],
                                &current_parse_output->lexer_output);

                    if (current_parse_output->lexer_output.messages.errors.size() != 0) {
                        // TODO: Error.
                    }

                    masonc::parser::parser_instance parser{ current_parse_output };
                }

                i += 1;
                goto LOOP;
            }

            // There is no (more) work, we either had a spurious wakeup, we didn't get any work,
            // or we already finished our work.
            work_index = work.size();
            file_queue_condition.wait(file_queue_mutex);
        }

        // Sync all the parse results.
        std::unique_lock<std::mutex> parse_output_lock{ parse_output_mutex };
        parse_output.insert(parse_output.end(),
                            buffered_parse_output.begin(),
                            buffered_parse_output.end());
    }

    void builder::split_work()
    {
        // Setting the average file size as a sort of "minimum amount of work"
        // should help counteract randomness of file sizes, such that some workers
        // don't only have a tiny file to work with while others have a huge file.
        // Instead, a worker will get multiple files to work with if they are relatively small
        // (provided that there is some variance in file sizes).
        u64 average_file_size = 0;
        for (u64 i = file_queue_first; i < file_sizes.size(); i += 1) {
            average_file_size += file_sizes[i];
        }
        average_file_size /= file_sizes.size() - file_queue_first;

        // How many bytes the current worker thread will be assigned to work with.
        u64 work_size = 0;

        u64 thread_index = 0;
        for (u64 i = file_queue_first; i < file_sizes.size(); i += 1)
        {
            all_work[thread_index].push_back(i);

            work_size += file_sizes[i];
            file_queue_first += 1;

            if (work_size > average_file_size) {
                work_size = 0;

                // Go to next worker, wrapping if there aren't many worker threads.
                if (thread_index < worker_thread_count - 1)
                    thread_index += 1;
                else
                    thread_index = 0;
            }
        }
    }

    std::vector<std::string> builder::concrete_file_paths(const std::vector<path>& sources) const
    {
        std::vector<std::string> file_paths;

        // Wild guess.
        file_paths.reserve(64);

        for (u64 i = 0; i < sources.size(); i += 1) {
            // TODO: Implement and call a function equivalent to "files_from_path"
            //       but that takes a vector of paths instead of one at a time?
            std::vector<std::string> files_in_path = files_from_path(sources[i], { ".mason", ".m" });
            file_paths.insert(file_paths.begin(), files_in_path.begin(), files_in_path.end());
        }

        return file_paths;
    }

    /*
    void build_object(std::vector<path> sources,
        robin_hood::unordered_set<std::string> additional_extensions)
    {
        additional_extensions.insert(".m");
        additional_extensions.insert(".mason");

        masonc::lexer::lexer_instance source_lexer;
        masonc::parser::parser_instance source_parser;
        //masonc::llvm::llvm_converter source_converter;

        std::vector<masonc::lexer::lexer_instance_output> lexer_outputs;
        std::vector<masonc::parser::parser_instance_output> parser_outputs;
        //std::vector<masonc::llvm::llvm_converter_output> converter_outputs;

        {
            std::cout << '\n' << "Lexing..." << std::endl;
            for (u64 i = 0; i < sources.size(); i += 1) {
                auto file_paths = files_from_path(sources[i], additional_extensions);

                for (u64 j = 0; j < file_paths.size(); j += 1) {
                    u64 current_file_length;
                    std::optional<char*> current_file = file_read(file_paths[j].c_str(),
                        1024u, &current_file_length);

                    if (!current_file)
                        goto END;

                    masonc::lexer::lexer_instance_output* current_lexer_output =
                        &lexer_outputs.emplace_back(masonc::lexer::lexer_instance_output{});

                    source_lexer.tokenize(current_file.value(),
                        current_file_length, current_lexer_output);

                    std::free(current_file.value());

                    if (current_lexer_output->messages.errors.size() > 0) {
                        current_lexer_output->messages.print_errors();
                        std::cout << std::endl;
                        goto END;
                    }
                }
            }

            std::cout << "Parsing..." << std::endl;
            for (u64 i = 0; i < lexer_outputs.size(); i += 1) {
                masonc::lexer::lexer_instance_output* current_lexer_output = &lexer_outputs[i];
                masonc::parser::parser_instance_output* current_parser_output =
                    &parser_outputs.emplace_back(masonc::parser::parser_instance_output{});

                source_parser.parse(current_lexer_output, current_parser_output);

                if (current_parser_output->messages.errors.size() > 0) {
                    current_parser_output->messages.print_errors();
                    std::cout << std::endl;
                    goto END;
                }
            }

            std::cout << "Generating code..." << std::endl;
            for (u64 i = 0; i < parser_outputs.size(); i += 1) {
                lexer_output* current_lexer_output = &lexer_outputs[i];
                parser_output* current_parser_output = &parser_outputs[i];
                llvm_converter_output* current_converter_output =
                    &converter_outputs.emplace_back(llvm_converter_output{});

                source_converter.convert(current_lexer_output, current_parser_output,
                    current_converter_output);

                if (current_converter_output->messages.errors.size() > 0) {
                    current_converter_output->messages.print_errors();
                    goto END;
                }
            }
        }

        END:
        source_parser.free();
    }
    */
}