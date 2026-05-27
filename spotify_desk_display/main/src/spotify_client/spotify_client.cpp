#include "spotify_client/spotify_client.h"
#include "esp_log.h"
#include "http_client/http_client.h"
#include "managers/nvs_manager.h"
#include <format>
#include <utils/encoding.h>
#include "cJSON.h"

SpotifyClient::SpotifyClient() {
    get_from_nvs();
    request_access_token();
}
SpotifyClient::~SpotifyClient() {}

std::optional<SpotifyPlayerState> SpotifyClient::get_player_state() {
    Http::Headers headers = get_headers();
    std::string body = "market=EN";

    HttpResponse response = get("https://api.spotify.com/v1/me/player?market=GB");
    if (response.is_error) {
        ESP_LOGE(TAG, "Failed to get player state - error in request");
        abort();
    }

    if (response.status_code == 200) {
        return SpotifyPlayerState::from_json(response.body);
    } else if (response.status_code == 204) {
        return std::nullopt;
    } else {
        ESP_LOGE(TAG, "Failed to get player state: status %d,  body: %s", response.status_code, response.body.c_str());
        abort();
    }
}


Http::Headers SpotifyClient::get_headers() {
    return {
        {"Authorization", std::format("Bearer {}", m_access_token)}
    };
}

HttpResponse SpotifyClient::get(const std::string& t_uri) {
    Http::Headers headers = get_headers();
    HttpResponse response = Http::get(t_uri.c_str(), headers);

    if (response.status_code == 401) {
        // access code might have expired, gen new and try again
        request_access_token();
        headers = get_headers();

        response = Http::get(t_uri.c_str(), headers);
        if (response.status_code == 401) {
            ESP_LOGE(SpotifyClient::TAG, "Refresh token invalid");
            abort();
        }
    }

    return response;
}

HttpResponse SpotifyClient::post(const std::string& t_uri, const std::string& t_body) {
    Http::Headers headers = get_headers();
    HttpResponse response = Http::post(t_uri.c_str(), headers, t_body);

    if (response.status_code == 401) {
        // access code might have expired, gen new and try again
        request_access_token();
        headers = get_headers();

        response = Http::post(t_uri.c_str(), headers, t_body);
        if (response.status_code == 401) {
            ESP_LOGE(SpotifyClient::TAG, "Refresh token invalid");
            abort();
        }
    }

    return response;

}

void SpotifyClient::get_from_nvs() {
    NvsManager nvs_manager = NvsManager::get_instance();

    m_client_id = nvs_manager.get_str("spotify_creds", "client_id");
    m_client_secret = nvs_manager.get_str("spotify_creds", "client_secret");
    m_refresh_token = nvs_manager.get_str("spotify_creds", "refresh_token");
}

void SpotifyClient::request_access_token() {
    const char* url = "https://accounts.spotify.com/api/token";
    Http::Headers headers = {
        {"Authorization", std::format("Basic {}", base64_encode(std::format("{}:{}", m_client_id, m_client_secret)))},
        {"Content-Type", "application/x-www-form-urlencoded"}

    };
    std::string body = std::format("grant_type=refresh_token&refresh_token={}", m_refresh_token);
    HttpResponse response = Http::post(url, headers, body);
    
    if (response.is_error) {
        ESP_LOGI(SpotifyClient::TAG, "Error while getting access token");
        abort();
    }

    if (response.status_code != 200) {
        ESP_LOGE(TAG, "Failed to get spotify access token, got %d reponse: %s", response.status_code, response.body.c_str());
        abort();
    }

    cJSON *json = cJSON_Parse(response.body.c_str());
    if (json == NULL) {
        ESP_LOGE(TAG, "Invalid JSON returned from spotify api");
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGE(TAG, "Error before: %s\n", error_ptr);
        }
        cJSON_Delete(json);
        abort();
    }

    const cJSON *access_token_json = cJSON_GetObjectItemCaseSensitive(json, "access_token");
    const cJSON *expires_in_json = cJSON_GetObjectItemCaseSensitive(json, "expires_in");

    if (cJSON_IsString(access_token_json) && (access_token_json->valuestring != NULL)) {
        m_access_token = std::string(access_token_json->valuestring);
    } else {
        ESP_LOGE(SpotifyClient::TAG, "Access token not found in response JSON");
        cJSON_Delete(json);
        abort();
    }

    if (cJSON_IsNumber(expires_in_json)) {
        m_expires_in = expires_in_json->valueint;
    } else {
        ESP_LOGW(SpotifyClient::TAG, "No expires in field found while getting access token");
    }

    cJSON_Delete(json);
}

