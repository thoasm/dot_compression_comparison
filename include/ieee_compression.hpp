#include <string>
#include <vector>


template <typename ValueType, typename StorageType>
struct ieee_compression_helper {
    ieee_compression_helper(std::size_t num_rows)
        : num_rows_{num_rows}, compressed_memory_(num_rows_)
    {}

    void compress(std::vector<ValueType>& in_out_memory)
    {
        assert((in_out_memory.size() >= num_rows_));

        for (std::size_t i = 0; i < num_rows_; ++i) {
            compressed_memory_[i] = static_cast<StorageType>(in_out_memory[i]);
        }
        for (std::size_t i = 0; i < num_rows_; ++i) {
            in_out_memory[i] = compressed_memory_[i];
        }
    }


private:
    std::size_t num_rows_;
    std::vector<StorageType> compressed_memory_;
};
