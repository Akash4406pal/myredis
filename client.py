import socket
import sys

def main():
    port = 9999
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            pass

    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('127.0.0.1', port))
        print(f"Connected to MiniRedis on port {port}!")
        print("Type commands (e.g. PING, SET key val, GET key, DEL key, EXISTS key) or 'exit'/'quit':\n")
    except Exception as e:
        print(f"Failed to connect to localhost:{port} -> {e}")
        return

    while True:
        try:
            cmd = input(f"127.0.0.1:{port}> ").strip()
            if not cmd:
                continue
            if cmd.lower() in ["exit", "quit"]:
                print("Disconnecting...")
                break

            s.sendall((cmd + "\n").encode("utf-8"))
            data = s.recv(4096).decode("utf-8")
            if not data:
                print("Server closed connection.")
                break
            print(data, end="")
        except (KeyboardInterrupt, EOFError):
            print("\nExiting...")
            break
        except Exception as e:
            print(f"Error: {e}")
            break

    s.close()

if __name__ == "__main__":
    main()
