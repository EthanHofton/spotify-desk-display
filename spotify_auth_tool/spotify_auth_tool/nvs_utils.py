from io import StringIO
import csv

from spotify_auth_tool.constants import (
    NVS_CSV_HEADERS,
    NVS_SPOTIFY_NAMESPACE,
    NVS_SPOTIFY_CLIENT_ID,
    NVS_SPOTIFY_CLIENT_SECRET,
    NVS_SPOTIFY_TOKEN_KEY,
    NVS_SPOTIFY_TOKEN_EXPIRE,
    TOKEN_FILE_EXPIRES_KEY,
    TOKEN_FILE_TOKEN_KEY
)

import json

from spotify_auth_tool.config import get_config


def build_nvs_csv(token_file: str, tmp_file) -> None:
    """
    Build NVS data csv from token file
    """

    token = None
    expires = None
    with open(token_file, "r") as tf:
        tf_json = json.load(tf)
        token = tf_json.get(TOKEN_FILE_TOKEN_KEY)
        expires = tf_json.get(TOKEN_FILE_EXPIRES_KEY)

    if token is None or expires is None:
        raise Exception("Invalid token file")

    config = get_config()

    csv_writer = csv.writer(tmp_file)

    csv_writer.writerow(NVS_CSV_HEADERS)
    csv_writer.writerow([
        NVS_SPOTIFY_NAMESPACE,
        'namespace',
        '',
        '',
    ])

    csv_data = {
        NVS_SPOTIFY_CLIENT_ID: config.spotify_client_id,
        NVS_SPOTIFY_CLIENT_SECRET: config.spotify_client_secret,
        NVS_SPOTIFY_TOKEN_KEY: token,
        NVS_SPOTIFY_TOKEN_EXPIRE: expires
    }

    csv_rows = [[key, 'data', 'string', value] for key, value in csv_data.items()]
    csv_writer.writerows(csv_rows)

    tmp_file.flush()
