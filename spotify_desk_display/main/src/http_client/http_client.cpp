#include "http_client/http_client.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"

namespace Http {

static constexpr const char* TAG = "Http";

HttpResponse get(const char* t_url, Headers t_headers) {
    esp_http_client_config_t config = {
        .url = t_url,
        .timeout_ms = 5000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    for (const auto& [key, value] : t_headers) {
        esp_http_client_set_header(client, key.c_str(), value.c_str());
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return { .is_error = true };
    }

    // Fetch headers, returns content-length (-1 if chunked/unknown)
    int content_length = esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "Status=%d, Content-Length=%d", status, content_length);

    // Read the body
    char buf[512];
    int total = 0, read_len = 0;

    std::string body;
    do {
        read_len = esp_http_client_read(client, buf, sizeof(buf) - 1);
        if (read_len < 0) break;
        buf[read_len] = '\0';
        body += buf;
        total += read_len;
    } while (read_len > 0);

    ESP_LOGI(TAG, "Total bytes read: %d", total);

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return {
        .status_code = static_cast<uint16_t>(status),
        .body = body,
        .is_error = false
    };
}

HttpResponse post(const char* t_url, Headers t_headers, const std::string& t_body) {
    esp_http_client_config_t config = {
        .url = t_url,
        .timeout_ms = 5000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);

    for (const auto& [key, value] : t_headers) {
        esp_http_client_set_header(client, key.c_str(), value.c_str());
    }

    esp_err_t err = esp_http_client_open(client, t_body.length());
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return { .is_error = true };
    }

    int written = esp_http_client_write(client, t_body.c_str(), t_body.length());
    if (written < 0) {
        ESP_LOGE(TAG, "Write failed");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return { .is_error = true };
    }

    int content_length = esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "Status=%d, Content-Length=%d", status, content_length);

    char buf[512];
    int total = 0, read_len = 0;
    std::string body;
    do {
        read_len = esp_http_client_read(client, buf, sizeof(buf) - 1);
        if (read_len < 0) break;
        buf[read_len] = '\0';
        body += buf;
        total += read_len;
    } while (read_len > 0);

    ESP_LOGI(TAG, "Total bytes read: %d", total);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return {
        .status_code = static_cast<uint16_t>(status),
        .body = body,
        .is_error = false
    };
}

}
