"""GLM API client for MiniCLI2."""

import json
import time
from typing import Optional, Any
import httpx
from rich.console import Console

from src.config import get_config

console = Console()


class GLMError(Exception):
    """GLM API error."""

    def __init__(self, message: str, code: Optional[int] = None):
        self.message = message
        self.code = code
        super().__init__(self.message)


class RateLimitError(GLMError):
    """Rate limit error (429)."""
    pass


class ProviderError(GLMError):
    """Provider error (5xx)."""
    pass


class GLMProvider:
    """GLM API provider with OpenAI-compatible format."""

    def __init__(
        self,
        api_key: Optional[str] = None,
        base_url: Optional[str] = None,
        model: Optional[str] = None,
        timeout_ms: Optional[int] = None,
        max_retries: Optional[int] = None,
    ):
        """Initialize GLM provider.

        Args:
            api_key: GLM API key.
            base_url: API base URL.
            model: Model name.
            timeout_ms: Request timeout in milliseconds.
            max_retries: Maximum number of retries.
        """
        config = get_config()

        self.api_key = api_key or config.api_key
        self.base_url = base_url or config.base_url
        self.model = model or config.model
        self.timeout_ms = timeout_ms or config.timeout_ms
        self.max_retries = max_retries or config.max_retries

        if not self.api_key:
            raise GLMError("API key is required. Set GLM_API_KEY environment variable or configure in .minicli2/config.json")

        self.client = httpx.Client(
            timeout=self.timeout_ms / 1000,
            headers={
                "Authorization": f"Bearer {self.api_key}",
                "Content-Type": "application/json",
            },
        )

    def _make_request(
        self,
        messages: list[dict],
        tools: Optional[list[dict]] = None,
        stream: bool = False,
    ) -> dict | AsyncGenerator[dict, None]:
        """Make API request to GLM.

        Args:
            messages: Chat messages.
            tools: Tools definitions.
            stream: Enable streaming.

        Returns:
            Response dict or stream generator.
        """
        url = f"{self.base_url}/chat/completions"

        payload: dict[str, Any] = {
            "model": self.model,
            "messages": messages,
            "stream": stream,
        }

        if tools:
            payload["tools"] = tools

        return self._request_with_retry(url, payload, stream)

    def _request_with_retry(
        self,
        url: str,
        payload: dict,
        stream: bool = False,
    ) -> dict | AsyncGenerator[dict, None]:
        """Make request with retry logic.

        Args:
            url: API URL.
            payload: Request payload.
            stream: Enable streaming.

        Returns:
            Response dict or stream generator.
        """
        last_error: Optional[Exception] = None

        for attempt in range(self.max_retries + 1):
            try:
                if stream:
                    # For streaming, run async generator in sync way
                    return self._run_stream(url, payload)
                else:
                    return self._sync_request(url, payload)
            except RateLimitError as e:
                last_error = e
                if attempt < self.max_retries:
                    wait_time = 2 ** attempt  # Exponential backoff
                    console.print(f"[yellow]Rate limited, waiting {wait_time}s before retry...[/yellow]")
                    time.sleep(wait_time)
                else:
                    raise
            except ProviderError as e:
                last_error = e
                if attempt < self.max_retries:
                    wait_time = 2 ** attempt
                    console.print(f"[yellow]Provider error {e.code}, waiting {wait_time}s before retry...[/yellow]")
                    time.sleep(wait_time)
                else:
                    raise
            except httpx.TimeoutException as e:
                last_error = GLMError(f"Request timeout: {e}")
                if attempt < self.max_retries:
                    wait_time = 2 ** attempt
                    console.print(f"[yellow]Request timeout, waiting {wait_time}s before retry...[/yellow]")
                    time.sleep(wait_time)
                else:
                    raise
            except httpx.HTTPError as e:
                raise GLMError(f"HTTP error: {e}")

        raise last_error or GLMError("Unknown error")

    def _sync_request(self, url: str, payload: dict) -> dict:
        """Make synchronous request.

        Args:
            url: API URL.
            payload: Request payload.

        Returns:
            Response dict.
        """
        response = self.client.post(url, json=payload)
        return self._handle_response(response)

    def _run_stream(self, url: str, payload: dict):
        """Run stream request synchronously using sync client.

        Args:
            url: API URL.
            payload: Request payload.

        Yields:
            Streamed response chunks.
        """
        with self.client.stream("POST", url, json=payload) as response:
            if response.status_code == 200:
                for line in response.iter_lines():
                    if line:
                        line = line.strip()
                        if line.startswith("data: "):
                            data = line[6:]
                            if data == "[DONE]":
                                break
                            yield json.loads(data)
            else:
                self._handle_response(response)

    def _handle_response(self, response: httpx.Response) -> dict:
        """Handle API response.

        Args:
            response: HTTP response.

        Returns:
            Response JSON.

        Raises:
            GLMError: On API error.
        """
        if response.status_code == 429:
            raise RateLimitError("Rate limit exceeded", 429)
        elif 500 <= response.status_code < 600:
            raise ProviderError(f"Provider error: {response.text}", response.status_code)

        try:
            data = response.json()
        except json.JSONDecodeError:
            raise GLMError(f"Invalid JSON response: {response.text}")

        if "error" in data:
            error = data["error"]
            raise GLMError(error.get("message", "Unknown error"), error.get("code"))

        return data

    def chat(
        self,
        messages: list[dict],
        tools: Optional[list[dict]] = None,
        stream: bool = False,
    ) -> dict | AsyncGenerator[dict, None]:
        """Chat with GLM.

        Args:
            messages: Chat messages.
            tools: Tools definitions.
            stream: Enable streaming.

        Returns:
            Response dict or stream generator.
        """
        return self._make_request(messages, tools, stream)

    def close(self) -> None:
        """Close the client."""
        self.client.close()


# Global provider instance
_provider: Optional[GLMProvider] = None


def get_provider() -> GLMProvider:
    """Get global provider instance."""
    global _provider
    if _provider is None:
        _provider = GLMProvider()
    return _provider
