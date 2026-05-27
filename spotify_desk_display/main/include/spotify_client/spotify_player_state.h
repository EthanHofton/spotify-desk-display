#pragma once

#include <string>
#include <variant>

struct SpotifyPlayerState {

    enum class RepeatState { OFF, TRACK, CONTEXT };
    enum class PlayingType { TRACK, EPISODE, AD, UNKNOWN };
    enum class ContextType { PLAYLIST, ALBUM, ARTIST, SHOW };

    struct TrackItem {
        char name[64];
        char artist_name[128];
        char album_name[64];
        char album_art_url[128];
        int duration_ms;
    };

    struct EpisodeItem {
        char name[64];
        char show_name[64];
        char image_url[128];
        int duration_ms;
    };

    // Playback
    bool is_playing;
    int progress_ms;
    int64_t timestamp;
    RepeatState repeat_state;
    bool shuffle_state;

    // Device
    char device_name[64];
    int volume_percent;
    bool supports_volume;

    // Item
    PlayingType currently_playing_type;
    std::variant<std::monostate, TrackItem, EpisodeItem> item;

    // Context
    ContextType context_type;

    static SpotifyPlayerState from_json(const std::string& t_json );
};
