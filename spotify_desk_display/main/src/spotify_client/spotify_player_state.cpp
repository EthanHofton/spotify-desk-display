#include "spotify_client/spotify_player_state.h"
#include "cJSON.h"
#include "esp_log.h"
#include <cstring>

SpotifyPlayerState SpotifyPlayerState::from_json(const std::string& t_json ) {
    cJSON* json = cJSON_Parse(t_json.c_str());
    SpotifyPlayerState state;

    if (json == NULL) {
        ESP_LOGE("SpotifyPlayerState", "Failed to parse JSON");
        return state;
    }

    cJSON* obj;

    // is playing
    obj = cJSON_GetObjectItemCaseSensitive(json, "is_playing");
    state.is_playing = cJSON_IsTrue(obj);

    // progress_ms
    obj = cJSON_GetObjectItemCaseSensitive(json, "progress_ms");
    if (obj != NULL && cJSON_IsNumber(obj)) {
        state.progress_ms = obj->valueint;
    }

    // timestamp
    obj = cJSON_GetObjectItemCaseSensitive(json, "timestamp");
    if (obj != NULL && cJSON_IsNumber(obj)) {
        state.timestamp = obj->valueint;
    }

    // repeat_state
    obj = cJSON_GetObjectItemCaseSensitive(json, "repeat_state");
    if (obj != NULL && cJSON_IsString(obj)) {
        if (strcmp(obj->valuestring, "off") == 0) {
            state.repeat_state = RepeatState::OFF;
        } else if (strcmp(obj->valuestring, "track") == 0) {
            state.repeat_state = RepeatState::TRACK;
        } else if (strcmp(obj->valuestring, "context") == 0) {
            state.repeat_state = RepeatState::CONTEXT;
        }
    }

    // shuffle_state
    obj = cJSON_GetObjectItemCaseSensitive(json, "shuffle_state");
    state.shuffle_state = cJSON_IsTrue(obj);

    cJSON* device = cJSON_GetObjectItemCaseSensitive(json, "device");
    if (device != NULL && cJSON_IsObject(device)) {
        // device_name
        obj = cJSON_GetObjectItemCaseSensitive(device, "name");
        if (obj != NULL && cJSON_IsString(obj) && obj->valuestring != NULL) {
            strncpy(state.device_name, obj->valuestring, sizeof(state.device_name) - 1);
            state.device_name[sizeof(state.device_name) - 1] = '\0';
        }

        // volume_percent
        obj = cJSON_GetObjectItemCaseSensitive(device, "volume_percent");
        if (obj != NULL && cJSON_IsNumber(obj)) {
            state.volume_percent = obj->valueint;
        }

        // supports_volume
        obj = cJSON_GetObjectItemCaseSensitive(device, "supports_volume");
        state.supports_volume = cJSON_IsTrue(obj);

    }

    // currently_playing_type
    obj = cJSON_GetObjectItemCaseSensitive(json, "currently_playing_type");
    if (obj != NULL && cJSON_IsString(obj)) {
        if (strcmp(obj->valuestring, "track") == 0) {
            state.currently_playing_type = PlayingType::TRACK;
        } else if (strcmp(obj->valuestring, "episode") == 0) {
            state.currently_playing_type = PlayingType::EPISODE;
        } else if (strcmp(obj->valuestring, "ad") == 0) {
            state.currently_playing_type = PlayingType::AD;
        } else if (strcmp(obj->valuestring, "unknown") == 0) {
            state.currently_playing_type = PlayingType::UNKNOWN;
        }
    }

    obj = cJSON_GetObjectItemCaseSensitive(json, "context");
    if (obj != NULL && cJSON_IsObject(obj)) {
        obj = cJSON_GetObjectItemCaseSensitive(obj, "type");
        if (obj != NULL && cJSON_IsString(obj)) {
            if (strcmp(obj->valuestring, "artist") == 0) {
                state.context_type = ContextType::ARTIST;
            } else if (strcmp(obj->valuestring, "playlist") == 0) {
                state.context_type = ContextType::PLAYLIST;
            } else if (strcmp(obj->valuestring, "album") == 0) {
                state.context_type = ContextType::ALBUM;
            } else if (strcmp(obj->valuestring, "show") == 0) {
                state.context_type = ContextType::SHOW;
            }
        }
    }

    cJSON* item = cJSON_GetObjectItemCaseSensitive(json, "item");
    if (item != NULL && cJSON_IsObject(item)) {
        cJSON* item_type = cJSON_GetObjectItemCaseSensitive(item, "type");

        if (item_type != NULL && cJSON_IsString(item_type) && strcmp(item_type->valuestring, "track") == 0) {
            TrackItem track_item;

            // track name — from track_obj
            cJSON* track_name = cJSON_GetObjectItemCaseSensitive(item, "name");
            if (track_name != NULL && cJSON_IsString(track_name) && track_name->valuestring != NULL) {
                strncpy(track_item.name, track_name->valuestring, sizeof(track_item.name) - 1);
                track_item.name[sizeof(track_item.name) - 1] = '\0';
            }

            // artists — from track_obj
            cJSON* artists = cJSON_GetObjectItemCaseSensitive(item, "artists");
            if (artists != NULL && cJSON_IsArray(artists)) {
                std::string artist_names;
                cJSON* artist = NULL;
                cJSON_ArrayForEach(artist, artists) {
                    if (artist != NULL && cJSON_IsObject(artist)) {
                        cJSON* artist_name = cJSON_GetObjectItemCaseSensitive(artist, "name");
                        if (artist_name != NULL && cJSON_IsString(artist_name) && artist_name->valuestring != NULL) {
                            if (!artist_names.empty()) artist_names += ", ";
                            artist_names += std::string(artist_name->valuestring);
                        }
                    }
                }
                if (!artist_names.empty()) {
                    strncpy(track_item.artist_name, artist_names.c_str(), sizeof(track_item.artist_name) - 1);
                    track_item.artist_name[sizeof(track_item.artist_name) - 1] = '\0';
                }
            }

            // album name — from album_obj
            cJSON* album_obj = cJSON_GetObjectItemCaseSensitive(item, "album");
            if (album_obj != NULL && cJSON_IsObject(album_obj)) {
                cJSON* album_name = cJSON_GetObjectItemCaseSensitive(album_obj, "name");
                if (album_name != NULL && cJSON_IsString(album_name) && album_name->valuestring != NULL) {
                    strncpy(track_item.album_name, album_name->valuestring, sizeof(track_item.album_name) - 1);
                    track_item.album_name[sizeof(track_item.album_name) - 1] = '\0';
                }

                // album art — from album_obj
                cJSON* images = cJSON_GetObjectItemCaseSensitive(album_obj, "images");
                if (images != NULL && cJSON_IsArray(images)) {
                    int image_count = cJSON_GetArraySize(images);
                    int image_index = (image_count > 1) ? 1 : 0;
                    cJSON* image = cJSON_GetArrayItem(images, image_index);
                    if (image != NULL && cJSON_IsObject(image)) {
                        cJSON* url = cJSON_GetObjectItemCaseSensitive(image, "url");
                        if (url != NULL && cJSON_IsString(url) && url->valuestring != NULL) {
                            strncpy(track_item.album_art_url, url->valuestring, sizeof(track_item.album_art_url) - 1);
                            track_item.album_art_url[sizeof(track_item.album_art_url) - 1] = '\0';
                        }
                    }
                }
            }

            // duration — from track_obj
            cJSON* duration = cJSON_GetObjectItemCaseSensitive(item, "duration_ms");
            if (duration != NULL && cJSON_IsNumber(duration)) {
                track_item.duration_ms = duration->valueint;
            }

            state.item = track_item;
        } else if (item_type != NULL && cJSON_IsString(item_type) && strcmp(item_type->valuestring, "episode") == 0) {
            // Episode
            EpisodeItem ep_item;

            // episode name
            cJSON* ep_name = cJSON_GetObjectItemCaseSensitive(item, "name");
            if (ep_name != NULL && cJSON_IsString(ep_name) && ep_name->valuestring != NULL) {
                strncpy(ep_item.name, ep_name->valuestring, sizeof(ep_item.name) - 1);
                ep_item.name[sizeof(ep_item.name) - 1] = '\0';
            }

            // show name
            cJSON* show = cJSON_GetObjectItemCaseSensitive(item, "show");
            if (show != NULL && cJSON_IsObject(show)) {
                cJSON* show_name = cJSON_GetObjectItemCaseSensitive(show, "name");
                if (show_name != NULL && cJSON_IsString(show_name) && show_name->valuestring != NULL) {
                    strncpy(ep_item.show_name, show_name->valuestring, sizeof(ep_item.show_name) - 1);
                    ep_item.show_name[sizeof(ep_item.show_name) - 1] = '\0';
                }
            }

            // image url
            cJSON* images = cJSON_GetObjectItemCaseSensitive(item, "images");
            if (images != NULL && cJSON_IsArray(images)) {
                int image_count = cJSON_GetArraySize(images);
                int image_index = (image_count > 1) ? 1 : 0;
                cJSON* image = cJSON_GetArrayItem(images, image_index);
                if (image != NULL && cJSON_IsObject(image)) {
                    cJSON* url = cJSON_GetObjectItemCaseSensitive(image, "url");
                    if (url != NULL && cJSON_IsString(url) && url->valuestring != NULL) {
                        strncpy(ep_item.image_url, url->valuestring, sizeof(ep_item.image_url) - 1);
                        ep_item.image_url[sizeof(ep_item.image_url) - 1] = '\0';
                    }
                }
            }

            // duration — from track_obj
            cJSON* duration = cJSON_GetObjectItemCaseSensitive(item, "duration_ms");
            if (duration != NULL && cJSON_IsNumber(duration)) {
                ep_item.duration_ms = duration->valueint;
            }


            state.item = ep_item;
        }
    }

    cJSON_Delete(json);
    return state;
}
