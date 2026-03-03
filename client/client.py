import argparse
import socket
import random
import string
import time
import sys

KEYS = ["tree", "sky", "water", "color", "name", "value", "key", "data"]

RETRY_DELAY = 2
ITERATIONS = 10_000
GET_PROBABILITY = 0.99


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Key-value client")
    parser.add_argument("--host", default="127.0.0.1", help="Server host")
    parser.add_argument("--port", type=int, default=12345, help="Server port")
    return parser.parse_args()


def random_value(length: int = 8) -> str:
    return "".join(random.choices(string.ascii_letters + string.digits, k=length))


def connect(host: str, port: int) -> socket.socket:
    while True:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(10)
            sock.connect((host, port))
            sock.settimeout(None)
            return sock
        except (socket.error, OSError) as e:
            print(f"Server unavailable: {e}. Retrying in {RETRY_DELAY}s...", file=sys.stderr)
            time.sleep(RETRY_DELAY)


def send_command(sock: socket.socket, command: str) -> str:
    sock.sendall((command + "\n").encode())
    lines = []
    buffer = ""
    while len(lines) < 3:
        buffer += sock.recv(4096).decode()
        if not buffer and not lines:
            raise ConnectionError("Connection closed by server")
        while "\n" in buffer:
            line, buffer = buffer.split("\n", 1)
            lines.append(line)
            if len(lines) >= 3:
                break
    return "\n".join(lines[:3])


def run_client(host: str, port: int) -> None:
    iteration = 0
    sock = None

    while iteration < ITERATIONS:
        try:
            if sock is None:
                sock = connect(host, port)
                print(f"Connected to {host}:{port}")

            key = random.choice(KEYS)
            if random.random() < GET_PROBABILITY:
                command = f"$get {key}"
            else:
                value = random_value()
                command = f"$set {key}={value}"

            response = send_command(sock, command)
            iteration += 1
            print(f"[{iteration}/{ITERATIONS}] {command} -> {response.strip()}")

        except (ConnectionError, BrokenPipeError, ConnectionResetError) as e:
            print(f"Connection lost: {e}. Reconnecting...", file=sys.stderr)
            if sock:
                try:
                    sock.close()
                except OSError:
                    pass
                sock = None

    if sock:
        sock.close()
    print(f"Completed {ITERATIONS} iterations.")


if __name__ == "__main__":
    args = parse_args()
    run_client(args.host, args.port)