from pydantic_settings import BaseSettings, SettingsConfigDict
from pydantic import field_validator
from functools import lru_cache

class Config(BaseSettings):
    spotify_client_id: str
    spotify_client_secret: str
    port: int = 8080
    model_config = SettingsConfigDict(env_file=".env")

    @field_validator("spotify_client_id", "spotify_client_secret")
    @classmethod
    def must_not_be_empty(cls, v: str, info) -> str:
        if not v or not v.strip():
            raise ValueError(f"{info.field_name} must not be empty")
        return v

@lru_cache
def get_config() -> Config:
    return Config() # type: ignore[call-arg]
