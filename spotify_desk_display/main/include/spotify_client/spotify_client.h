#pragma once

#include "http_client/http_client.h"
#include "spotify_player_state.h"
#include <optional>
#include <string>

class SpotifyClient {
public:
    static constexpr const char* TAG = "SpotifyClient";

    SpotifyClient();
    ~SpotifyClient();

    std::optional<SpotifyPlayerState> get_player_state();

private:

    Http::Headers get_headers();

    void get_from_nvs();
    void request_access_token();

    HttpResponse get(const std::string& t_uri);
    HttpResponse post(const std::string& t_uri, const std::string& t_body);

private:

    std::string m_client_id;
    std::string m_client_secret;
    std::string m_refresh_token;
    std::string m_access_token;
    uint16_t m_aquired_at;
    uint16_t m_expires_in;
};
