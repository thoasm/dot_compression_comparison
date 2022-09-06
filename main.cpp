#include <iostream>
#include <fstream>
#include <string>

#include <libpressio_ext/cpp/json.h>
#include <libpressio_ext/cpp/libpressio.h>
#include <libpressio_meta.h>  //provides frsz
#include <fstream>
#include <nlohmann/json.hpp>

#include "directories.hpp"


template <typename ValueType, typename StorageType>
struct compression_helper {
    compression_helper(bool use_compr, std::string compressor,
                       std::size_t num_rows, std::size_t num_vecs,
                       std::string lp_config)
        : use_compr_{use_compr},
          compressor_(compressor),
          num_rows_{num_rows},
          plibrary_{},
          pc_{},
          in_temp_{},
          out_temp_{},
          p_data_vec_(use_compr_ ? 1 : 0),
          metrics_plugins_{"time",     "size",     "error_stat",
                           "clipping", "data_gap", "write_debug_inputs"}
    {
        using namespace std::string_literals;
        if (use_compr_) {
            libpressio_register_all();
            std::ifstream pressio_input_file(lp_config);
            nlohmann::json j;
            pressio_input_file >> j;
            pressio_options options_from_file(static_cast<pressio_options>(j));
            pressio library;
            pc_ = library.get_compressor("pressio");
            pc_->set_options({
                {"pressio:metric", "composite"s},
                {"composite:plugins", metrics_plugins_},
                // {"write_debug_inputs:write_input", true},
                // {"write_debug_inputs:display_paths", true},
                // {"write_debug_inputs:io", "posix"},
            });
            pc_->set_name("pressio");
            pc_->set_options(options_from_file);
            std::cerr << pc_->get_options() << std::endl;
            const auto pressio_type = std::is_same<ValueType, float>::value
                                          ? pressio_float_dtype
                                          : pressio_double_dtype;
            for (std::size_t i = 0; i < p_data_vec_.size(); ++i) {
                p_data_vec_[i] =
                    pressio_data::owning(pressio_type, {num_rows_});
            }
            in_temp_ = pressio_data::owning(pressio_type, {num_rows_});
            out_temp_ = pressio_data::owning(pressio_type, {num_rows_});
        }
    }

    void compress(std::size_t krylov_idx,
                  gko::cb_gmres::Range3dHelper<ValueType, StorageType>& rhelper)
    {
        if (!use_compr_) {
            return;
        }
        GKO_ASSERT(rhelper.get_range().length(2) == 1);

        const auto exec = rhelper.get_bases().get_executor().get();
        const auto host_exec = exec->get_master().get();

        // Reinterpret_cast necessary for type check if no compressor is used
        auto raw_krylov_base = reinterpret_cast<ValueType*>(
            rhelper.get_bases().get_data() + krylov_idx * num_rows_);
        host_exec->copy_from(exec, num_rows_, raw_krylov_base,
                             reinterpret_cast<ValueType*>(in_temp_.data()));
        pc_->compress(&in_temp_, &p_data_vec_[0]);
        pc_->decompress(&p_data_vec_[0], &out_temp_);
        exec->copy_from(host_exec, num_rows_,
                        reinterpret_cast<const ValueType*>(out_temp_.data()),
                        raw_krylov_base);
    }

    void print_metrics() const
    {
        if (false && use_compr_) {
            std::cout << pc_->get_metrics_results() << '\n';
        }
    }


private:
    std::string compressor_;
    bool use_compr_;
    std::size_t num_rows_;
    pressio plibrary_;
    pressio_compressor pc_;
    pressio_data in_temp_;
    pressio_data out_temp_;
    std::vector<pressio_data> p_data_vec_;
    std::vector<std::string> metrics_plugins_;
};

int main() {
    std::cout << "Configuration:\n";
}
