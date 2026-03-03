"""
Reconnection test: server, 10 clients, after 5 sec — one more client + kill server,
after 1 sec — restart server.
All processes in separate consoles. Windows stay open for manual closure.
"""

import subprocess
import sys
import time
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent
if sys.platform == "win32":
    SERVER_DIR = PROJECT_ROOT / "server" / "build" / "Debug"
else:
    SERVER_DIR = PROJECT_ROOT / "server" / "build"
SERVER_EXE = "server.exe" if sys.platform == "win32" else "server"
CLIENT_SCRIPT = PROJECT_ROOT / "client" / "client.py"

NUM_CLIENTS = 10
INITIAL_DELAY = 5
RESTART_DELAY = 1


def main() -> None:
    server_dir = SERVER_DIR
    server_path = server_dir / SERVER_EXE
    if not server_path.exists():
        alt_dir = PROJECT_ROOT / "server" / "build"
        alt_path = alt_dir / SERVER_EXE
        if alt_path.exists():
            server_path = alt_path
            server_dir = alt_dir
        else:
            print(f"Error: server not found: {server_path}", file=sys.stderr)
            sys.exit(1)

    print("1. Starting server in separate console...")
    server = subprocess.Popen(
        [str(server_path)],
        cwd=str(server_dir),
        creationflags=subprocess.CREATE_NEW_CONSOLE if sys.platform == "win32" else 0,
    )
    time.sleep(2)

    if server.poll() is not None:
        print("Error: server failed to start", file=sys.stderr)
        sys.exit(1)

    print("2. Starting 10 clients in separate consoles...")
    for i in range(NUM_CLIENTS):
        subprocess.Popen(
            ["cmd", "/c", "start", f"Client {i + 1}", "cmd", "/k", "python client\\client.py --host 127.0.0.1 --port 12345"],
            cwd=str(PROJECT_ROOT),
        )
        time.sleep(0.3)

    print(f"3. Waiting {INITIAL_DELAY} sec, then client 11 + kill server...")
    time.sleep(INITIAL_DELAY)

    print("   Starting client 11...")
    subprocess.Popen(
        ["cmd", "/c", "start", "Client 11", "cmd", "/k", "python client\\client.py --host 127.0.0.1 --port 12345"],
        cwd=str(PROJECT_ROOT),
    )

    print("   Stopping server...")
    server.terminate()
    try:
        server.wait(timeout=3)
    except subprocess.TimeoutExpired:
        server.kill()

    print(f"4. Waiting {RESTART_DELAY} sec, then restarting server...")
    time.sleep(RESTART_DELAY)

    print("5. Starting server in separate console...")
    subprocess.Popen(
        [str(server_path)],
        cwd=str(server_dir),
        creationflags=subprocess.CREATE_NEW_CONSOLE if sys.platform == "win32" else 0,
    )

    print("Done. Clients continue running (reconnect on disconnect).")
    print("Close client and server windows manually.")


if __name__ == "__main__":
    main()
