import typer
import uvicorn
import threading
import json
from fastapi import FastAPI
from contextlib import asynccontextmanager
from spotify_auth_tool.config import get_config
from spotify_auth_tool.nvs_utils import build_nvs_csv
from spotify_auth_tool.spotify_client import generate_auth_link, request_refresh_token
from asyncio import Event
import asyncio
from tempfile import NamedTemporaryFile
import sys
import os

    # setup parttool api
idf_path = os.environ["IDF_PATH"]
if idf_path is None:
    raise Exception("No IDF_PATH, did you call get_idf?")
parttool_dir = os.path.join(idf_path, "components", "partition_table")
sys.path.append(parttool_dir)
# from parttool import * # type: ignore
from parttool import *

app = typer.Typer()
state_true: str | None = None

@app.command()
def auth():
    """
    Get spotify refresh token
    """
    event = Event()

    def on_ready():
        global state_true
        print("server up")
        auth_link, state_true = generate_auth_link()
        print(f"Login with: {auth_link}")
        print()
        print(f"Generated state: {state_true}")

    @asynccontextmanager
    async def lifespan(api: FastAPI):
        threading.Thread(target=on_ready).start()

        yield

        while not server.should_exit:
            if event.is_set():
                break

            await asyncio.sleep(0.1)

        server.should_exit = True

    api = FastAPI(lifespan=lifespan)

    @api.get("/callback")
    async def callback(
            state: str,
            code: str | None = None,
            error: str | None = None
        ):
        global state_true
        print(f"Got code from Spotify: {code}")
        if state != state_true:
            print(f"State missmatch: {state} != {state_true}")
            event.set()
            return {"status": "error", "error": "state does not match"}

        if error:
            print(f"Auth failed: {error}")
            event.set()
            return {"status": "error", "error": error}

        if not code:
            print(f"No code returned")
            return {"status": "error", "error": "no code provided"}

        token, expires = request_refresh_token(code)
        with open("token.json", "w") as f:
            json.dump({"refresh_token": token, "expires_in": expires}, f, indent=2)
        
        print(f"Got refresh token: {token}")
        print(f"expires in: {expires}")
        
        event.set()
        return {"status": "ok"}

    app_config = get_config()
    uvicorn_config = uvicorn.Config(api, host="127.0.0.1", port=app_config.port)
    server = uvicorn.Server(uvicorn_config)

    server.run()

@app.command()
def flash(
    token_file: str,
    port: str,
    baud: int = 115200,
    encrypt: bool = False
):
    if encrypt:
        raise NotImplementedError("NVS Encryption not implamented yet")

    with NamedTemporaryFile("w", suffix=".csv") as f:
        build_nvs_csv(token_file, f)

        # get NVS partion size
        target = ParttoolTarget(port, baud)
        storage = target.get_partition_info(PartitionName("nvs"))
        print(storage)
        print(storage.size)

def main():
    app()
