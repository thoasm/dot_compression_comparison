#pragma once

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <libpressio_ext/cpp/json.h>
#include <libpressio_ext/cpp/libpressio.h>
#include <libpressio_meta.h>  //provides frsz
#include <nlohmann/json.hpp>


template <typename ValueType>
struct pressio_compression_helper {
    pressio_compression_helper(std::size_t num_rows, std::string lp_config)
        : num_rows_{num_rows},
          plibrary_{},
          pc_{},
          in_temp_{},
          out_temp_{},
          compressed_memory_{},
          metrics_plugins_{"time",     "size",     "error_stat",
                           "clipping", "data_gap", "write_debug_inputs"}
    {
        using namespace std::string_literals;
        libpressio_register_all();
        std::ifstream pressio_input_file(lp_config);
        nlohmann::json j;
        pressio_input_file >> j;
        pressio_options options_from_file(static_cast<pressio_options>(j));
        pressio library;
        pc_ = library.get_compressor("pressio");
        pc_->set_options({
            {"pressio:metric", "size"s},
            {"composite:plugins", metrics_plugins_},
            //{"write_debug_inputs:write_input", true},
            //{"write_debug_inputs:display_paths", true},
            //{"write_debug_inputs:io", "posix"},
        });
        pc_->set_name("pressio");
        pc_->set_options(options_from_file);
        // std::cerr << pc_->get_options() << std::endl;
        const auto pressio_type = std::is_same<ValueType, float>::value
                                      ? pressio_float_dtype
                                      : pressio_double_dtype;
        in_temp_ = pressio_data::owning(pressio_type, {num_rows_});
        out_temp_ = pressio_data::owning(pressio_type, {num_rows_});
        compressed_memory_ = pressio_data::owning(pressio_type, {0});
    }

    void compress(std::vector<ValueType>& in_out_memory)
    {
        assert((in_out_memory.size() >= num_rows_));

        // Reinterpret_cast necessary for type check if no compressor is used
        std::memcpy(in_temp_.data(), in_out_memory.data(),
                    num_rows_ * sizeof(ValueType));
        pc_->compress(&in_temp_, &compressed_memory_);
        // std::cout << pc_->get_metrics_results() << '\n';
        // std::cerr << pc_->get_options() << std::endl;
        pc_->decompress(&compressed_memory_, &out_temp_);
        std::memcpy(in_out_memory.data(), out_temp_.data(),
                    num_rows_ * sizeof(ValueType));
    }

    void print_metrics() const
    {
        std::cout << pc_->get_metrics_results() << '\n';
        // std::cout << pc_->get_plugins() << '\n';
    }

    double get_compression_ratio() const
    {
        double ratio{};
        pc_->get_metrics_results().get("/pressio/size:size:compression_ratio",
                                       &ratio);
        return ratio;
    }


private:
    std::size_t num_rows_;
    pressio plibrary_;
    pressio_compressor pc_;
    pressio_data in_temp_;
    pressio_data out_temp_;
    pressio_data compressed_memory_;
    std::vector<std::string> metrics_plugins_;
};

