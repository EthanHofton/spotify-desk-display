#include "utils/encoding.h"

#include "mbedtls/base64.h"

std::string base64_encode(const std::string& t_in) {
    size_t output_len = 0;

    // Calculate required output buffer size
    mbedtls_base64_encode(nullptr, 0, &output_len,
        reinterpret_cast<const unsigned char*>(t_in.data()), t_in.size());

    std::string output(output_len, '\0');

    mbedtls_base64_encode(
        reinterpret_cast<unsigned char*>(output.data()),
        output_len,
        &output_len,
        reinterpret_cast<const unsigned char*>(t_in.data()),
        t_in.size()
    );

    // mbedtls appends a null terminator and counts it in output_len — trim it
    if (!output.empty() && output.back() == '\0') {
        output.pop_back();
    }

    return output;
}
