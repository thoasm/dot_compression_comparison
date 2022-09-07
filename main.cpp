#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>


#include <quadmath.h>
#include <cmath>


#include "compression_helper.hpp"
#include "directories.hpp"
#include "ieee_compression.hpp"


template <typename ArithmeticType, typename StorageType>
ArithmeticType dot(std::size_t num_elems, const StorageType* a,
                   const StorageType* b)
{
    ArithmeticType result = 0;
    for (std::size_t i = 0; i < num_elems; ++i) {
        result =
            result + static_cast<ArithmeticType>(a[i]) * ArithmeticType(b[i]);
        /*
        if (a[i] < 0 || b[i] < 0) {
            std::cerr << "Problem at index " << i << ": " << a[i] << ' ' << b[i]
                      << '\n';
        }
        */
    }
    return result;
}


template <typename ValueType>
void run_error_analysis(const std::vector<ValueType>& vec_a,
                        const std::vector<ValueType>& vec_b,
                        std::vector<ValueType>& c_vec_a,
                        std::vector<ValueType>& c_vec_b)
{
    assert((vec_a.size() == vec_b.size()));
    const auto num_elems = vec_a.size();
    const auto exact_res = static_cast<ValueType>(
        dot<__float128>(num_elems, vec_a.data(), vec_b.data()));

    std::vector<std::string> compression_json_files;
    for (auto config_path :
         std::filesystem::directory_iterator(DEFAULT_COMPRESSION_DIR)) {
        compression_json_files.emplace_back(config_path.path().string());
    }
    std::sort(compression_json_files.begin(), compression_json_files.end());
    for (auto config_file : compression_json_files) {
        /*
        benchmarks.emplace_back();
        benchmarks.back().name = str_pre + "lp" + str_post;
        benchmarks.back().settings = default_ss;
        benchmarks.back().settings.storage_prec =
            gko::solver::cb_gmres::storage_precision::use_sz;
        benchmarks.back().settings.lp_config = config_file;
        */
        auto begin_file_name = config_file.rfind('/');
        begin_file_name =
            begin_file_name == std::string::npos ? 0 : begin_file_name + 1;
        const auto file_name = config_file.substr(
            begin_file_name, config_file.size() - begin_file_name - 5);

        c_vec_a = vec_a;
        c_vec_b = vec_b;
        pressio_compression_helper<ValueType> helper(num_elems, config_file);
        helper.compress(c_vec_a);
        helper.compress(c_vec_b);
        auto local_res =
            dot<ValueType>(num_elems, c_vec_a.data(), c_vec_b.data());
        std::cout << file_name << " diff: " << std::abs(exact_res - local_res)
                  << '\n';
    }
    {
        c_vec_a = vec_a;
        c_vec_b = vec_b;
        ieee_compression_helper<ValueType, double> helper(num_elems);
        helper.compress(c_vec_a);
        helper.compress(c_vec_b);
        auto local_res =
            dot<ValueType>(num_elems, c_vec_a.data(), c_vec_b.data());
        std::cout << "double"
                  << " diff: " << std::abs(exact_res - local_res) << '\n';
    }
    {
        c_vec_a = vec_a;
        c_vec_b = vec_b;
        ieee_compression_helper<ValueType, float> helper(num_elems);
        helper.compress(c_vec_a);
        helper.compress(c_vec_b);
        auto local_res =
            dot<ValueType>(num_elems, c_vec_a.data(), c_vec_b.data());
        std::cout << "float"
                  << " diff: " << std::abs(exact_res - local_res) << '\n';
    }
}


int main()
{
    const std::size_t num_elems{10000};
    using value_type = double;
    std::vector<value_type> vec_a(num_elems);
    std::vector<value_type> vec_b(num_elems);
    std::vector<value_type> c_vec_a(num_elems);
    std::vector<value_type> c_vec_b(num_elems);
    for (std::size_t i = 0; i < num_elems; ++i) {
        const auto a_val =
            std::sin((static_cast<value_type>(i) / num_elems) * M_PI);
        const auto b_val = a_val;
        // const auto a_val = r_dist(r_engine);
        // const auto b_val = r_dist(r_engine);
        vec_a[i] = a_val;
        vec_b[i] = b_val;
    }

    std::cout << "a and b use sinus for value generation\n";
    std::cout << std::scientific << std::setprecision(6);
    run_error_analysis(vec_a, vec_b, c_vec_a, c_vec_b);

    std::random_device r_dev;
    std::default_random_engine r_engine(r_dev());
    std::uniform_real_distribution<double> r_dist(0, 1);
    for (std::size_t i = 0; i < num_elems; ++i) {
        const auto a_val = r_dist(r_engine);
        const auto b_val = r_dist(r_engine);
        vec_a[i] = a_val;
        vec_b[i] = b_val;
    }

    std::cout << "a and b are randomly generated with unit distribution ["
              << r_dist.a() << ", " << r_dist.b() << ")\n";
    run_error_analysis(vec_a, vec_b, c_vec_a, c_vec_b);
}
