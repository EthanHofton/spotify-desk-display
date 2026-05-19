from spotify_auth_tool.config import get_config
from spotify_auth_tool.util import random_string
from urllib.parse import urlencode
import httpx

import base64

SPOTIFY_SCOPES = [
    # Core MVP
    "user-read-playback-state",
    "user-modify-playback-state",
    "user-read-currently-playing",
    "user-read-private",
    "playlist-read-private",
    # Future
    "user-top-read",
    "user-read-recently-played",
    "user-library-read",
    # "user-personalized",
]


def generate_auth_link() -> tuple[str, str]:
    """
        Generate a Spotify auth link for the user to click

        Returns:
            url: url for the user to authenticate
            state: the state value used to prevent cross-site request forgery
    """
    config = get_config()

    state = random_string(16)
    callback_url = f"http://127.0.0.1:{config.port}/callback"

    base = 'https://accounts.spotify.com/authorize?'
    params = {
        'response_type': 'code',
        'client_id': config.spotify_client_id,
        'scope': ' '.join(SPOTIFY_SCOPES),
        'redirect_uri': callback_url,
        'state': state
    }

    return base + urlencode(params), state

def request_refresh_token(code: str) -> tuple[str, str]:
    """
        Get a refresh token with code

        Returns:
            refresh_token: the spotify refresh_token
            expires_in: the time period in seconds of which the token is valid for
    """
    config = get_config()
    callback_url = f"http://127.0.0.1:{config.port}/callback"
    base64_secret = base64.b64encode(f"{config.spotify_client_id}:{config.spotify_client_secret}".encode("utf-8")).decode("utf-8")

    
    url = 'https://accounts.spotify.com/api/token'
    body = {
        'grant_type': 'authorization_code',
        'code': code,
        'redirect_uri': callback_url # validation only
    }
    headers = {
        'content-type': 'application/x-www-form-urlencoded',
        'Authorization': f'Basic {base64_secret}'
    }

    response = httpx.post(
        url,
        headers=headers,
        data=body
    )

    response.raise_for_status()
    if response.status_code == 200:
        response_json = response.json()
        return response_json["refresh_token"], response_json["expires_in"]
    else:
        raise Exception(f"Could not fetch refresh token: non 200 status code: {response.status_code}")

