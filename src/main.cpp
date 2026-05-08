#include "nibbler.hpp"
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <cctype>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int currentLibrary = SDL3;

typedef void* (*create_t)(int, int);
typedef void (*destroy_t)(void*);
typedef void (*display_t)(void*, const Game&);
typedef int (*input_t)(void*);

enum AppMode {
    MODE_SOLO,
    MODE_LOCAL,
    MODE_ONLINE_HOST,
    MODE_ONLINE_CLIENT
};

enum MessageType : uint8_t {
    MSG_HANDSHAKE = 1,
    MSG_INPUT = 2,
    MSG_QUIT = 3
};

static bool isNumber(const char* s) {
    if (!s || *s == '\0') return false;
    for (const char* p = s; *p != '\0'; ++p) {
        if (!std::isdigit(*p)) return false;
    }
    return true;
}

static bool send_all(int socket_fd, const void* data, size_t length) {
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(data);
    while (length > 0) {
        ssize_t sent = send(socket_fd, buffer, length, 0);
        if (sent <= 0) {
            return false;
        }
        buffer += sent;
        length -= sent;
    }
    return true;
}

static bool recv_all(int socket_fd, void* data, size_t length) {
    uint8_t* buffer = reinterpret_cast<uint8_t*>(data);
    while (length > 0) {
        ssize_t received = recv(socket_fd, buffer, length, MSG_WAITALL);
        if (received <= 0) {
            return false;
        }
        buffer += received;
        length -= received;
    }
    return true;
}

static bool setNonBlocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) >= 0;
}

static int create_server_socket(int port, int& actual_port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return -1;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 1) < 0) {
        close(server_fd);
        return -1;
    }

    socklen_t addrlen = sizeof(addr);
    if (getsockname(server_fd, reinterpret_cast<sockaddr*>(&addr), &addrlen) < 0) {
        close(server_fd);
        return -1;
    }

    actual_port = ntohs(addr.sin_port);
    return server_fd;
}

static int create_client_socket(const char* host, int port) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        close(client_fd);
        return -1;
    }

    if (connect(client_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(client_fd);
        return -1;
    }

    return client_fd;
}

static bool send_handshake(int socket_fd, uint32_t seed, int width, int height) {
    uint8_t buffer[9];
    buffer[0] = MSG_HANDSHAKE;
    uint32_t net_seed = htonl(seed);
    memcpy(&buffer[1], &net_seed, sizeof(net_seed));
    uint16_t net_height = htons(static_cast<uint16_t>(height));
    uint16_t net_width = htons(static_cast<uint16_t>(width));
    memcpy(&buffer[5], &net_height, sizeof(net_height));
    memcpy(&buffer[7], &net_width, sizeof(net_width));
    return send_all(socket_fd, buffer, sizeof(buffer));
}

static bool recv_handshake(int socket_fd, uint32_t& seed, int& width, int& height) {
    uint8_t buffer[9];
    if (!recv_all(socket_fd, buffer, sizeof(buffer))) {
        return false;
    }
    if (buffer[0] != MSG_HANDSHAKE) {
        return false;
    }
    uint32_t net_seed;
    memcpy(&net_seed, &buffer[1], sizeof(net_seed));
    seed = ntohl(net_seed);
    uint16_t net_height;
    uint16_t net_width;
    memcpy(&net_height, &buffer[5], sizeof(net_height));
    memcpy(&net_width, &buffer[7], sizeof(net_width));
    height = ntohs(net_height);
    width = ntohs(net_width);
    return true;
}

static bool send_input_message(int socket_fd, int input_code) {
    uint8_t buffer[2];
    buffer[0] = MSG_INPUT;
    buffer[1] = static_cast<uint8_t>(input_code & 0xFF);
    return send_all(socket_fd, buffer, sizeof(buffer));
}

static bool send_quit_message(int socket_fd) {
    uint8_t buffer[1] = { MSG_QUIT };
    return send_all(socket_fd, buffer, sizeof(buffer));
}

static bool drain_network(int socket_fd, std::vector<uint8_t>& buffer, std::vector<int>& remote_inputs, bool& remote_quit) {
    uint8_t temp[256];
    while (true) {
        ssize_t received = recv(socket_fd, temp, sizeof(temp), MSG_DONTWAIT);
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            return false;
        }
        if (received == 0) {
            remote_quit = true;
            return false;
        }
        buffer.insert(buffer.end(), temp, temp + received);
    }

    size_t pos = 0;
    while (pos < buffer.size()) {
        uint8_t type = buffer[pos];
        if (type == MSG_INPUT) {
            if (pos + 2 > buffer.size()) break;
            int input_code = static_cast<int8_t>(buffer[pos + 1]);
            remote_inputs.push_back(input_code);
            pos += 2;
            continue;
        }
        if (type == MSG_QUIT) {
            remote_quit = true;
            pos += 1;
            continue;
        }
        pos += 1;
    }

    if (pos > 0) {
        buffer.erase(buffer.begin(), buffer.begin() + pos);
    }
    return true;
}

int main(int argc, char** argv) {
    if (!((argc == 2) || (argc == 4) || (argc == 5))) {
        std::cerr << "Usage: " << argv[0] << " <height> <width> <sfml/sdl3/gl> [1|2|online]" << std::endl;
        std::cerr << "       " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    const char* available_libraries[] = {"sfml", "sdl3", "gl"};
    const char* selected_library = "sdl3";
    bool valid_library = true;
    AppMode mode = MODE_SOLO;
    int width = 0;
    int height = 0;
    int nbPlayer = 1;
    int network_socket = -1;
    int server_socket = -1;
    int listen_port = 0;
    uint32_t seed = 0;

    if (argc == 2) {
        if (!isNumber(argv[1])) {
            std::cerr << "Error: When using a port, the single argument must be a port number." << std::endl;
            return 1;
        }
        int port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Error: Invalid port number." << std::endl;
            return 1;
        }
        mode = MODE_ONLINE_CLIENT;
        network_socket = create_client_socket("127.0.0.1", port);
        if (network_socket < 0) {
            std::cerr << "Error: Could not connect to host on port " << port << "." << std::endl;
            return 1;
        }
    } else {
        height = std::atoi(argv[1]);
        width = std::atoi(argv[2]);
        selected_library = argv[3];
        valid_library = false;
        for (int i = 0; i < 3; i++) {
            if (strcmp(selected_library, available_libraries[i]) == 0) {
                valid_library = true;
                break;
            }
        }
        if (!valid_library) {
            std::cerr << "Error: Invalid library. Available options are: sfml, sdl3, gl" << std::endl;
            return 1;
        }
        currentLibrary = (selected_library[0] == 's' && selected_library[1] == 'f') ? SFML : 
                         (selected_library[0] == 's' && selected_library[1] == 'd') ? SDL3 : GL;

        if (width <= 9 || height <= 9 || width > 100 || height > 100) {
            std::cerr << "Error: Invalid width or height" << std::endl;
            return 1;
        }

        if (argc == 5) {
            if (strcmp(argv[4], "online") == 0) {
                mode = MODE_ONLINE_HOST;
                nbPlayer = 2;
            } else if (strcmp(argv[4], "2") == 0) {
                mode = MODE_LOCAL;
                nbPlayer = 2;
            } else if (strcmp(argv[4], "1") == 0) {
                mode = MODE_SOLO;
                nbPlayer = 1;
            } else {
                std::cerr << "Error: Invalid player option. Available options are: 1, 2, online" << std::endl;
                return 1;
            }
        } else {
            mode = MODE_SOLO;
            nbPlayer = 1;
        }

        // Ensure nbPlayer is consistent for local mode
        if (mode == MODE_LOCAL) {
            nbPlayer = 2;
        }
    }

    void* gui = nullptr;
    void* handle = nullptr;
    create_t create_gui = nullptr;
    destroy_t destroy_gui = nullptr;
    display_t display_gui = nullptr;
    input_t input_gui = nullptr;

    auto load_lib = [&](const char* lib_so, int currentLibrary) -> bool {
        const char* expected_suffix = (currentLibrary == SFML) ? "sfml" : (currentLibrary == SDL3) ? "sdl3" : "gl";
        const std::string create_symbol = std::string("create_gui_") + expected_suffix;
        const std::string destroy_symbol = std::string("destroy_gui_") + expected_suffix;
        const std::string display_symbol = std::string("display_gui_") + expected_suffix;
        const std::string input_symbol = std::string("input_gui_") + expected_suffix;
        if (handle) {
            destroy_gui(gui);
            dlclose(handle);
            handle = nullptr;
            gui = nullptr;
        }
        handle = dlopen(lib_so, RTLD_LAZY);
        if (!handle) {
            std::cerr << "Error loading " << lib_so << ": " << dlerror() << std::endl;
            return false;
        }

        create_gui = reinterpret_cast<create_t>(dlsym(handle, create_symbol.c_str()));
        destroy_gui = reinterpret_cast<destroy_t>(dlsym(handle, destroy_symbol.c_str()));
        display_gui = reinterpret_cast<display_t>(dlsym(handle, display_symbol.c_str()));
        input_gui = reinterpret_cast<input_t>(dlsym(handle, input_symbol.c_str()));
        if (!create_gui || !destroy_gui || !display_gui || !input_gui) {
            std::cerr << "Error: Missing symbols in library" << std::endl;
            dlclose(handle);
            handle = nullptr;
            return false;
        }
        if (mode != MODE_ONLINE_CLIENT) {
            gui = create_gui(width, height);
            if (!gui) {
                std::cerr << "Error: Failed to create GUI." << std::endl;
                dlclose(handle);
                handle = nullptr;
                return false;
            }
        }
        return true;
    };

    const char* gui_lib = (currentLibrary == SFML) ? "./lib_sfml.so" :
                          (currentLibrary == SDL3) ? "./lib_sdl3.so" : "./lib_gl.so";
    if (!load_lib(gui_lib, currentLibrary)) {
        return 1;
    }

    Game game;
    if (mode == MODE_ONLINE_HOST) {
        server_socket = create_server_socket(0, listen_port);
        if (server_socket < 0) {
            std::cerr << "Error: Unable to open listening socket." << std::endl;
            return 1;
        }
        std::cerr << "Online host listening on port " << listen_port << std::endl;
        std::cerr << "Waiting for the other player to connect..." << std::endl;
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        network_socket = accept(server_socket, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
        if (network_socket < 0) {
            std::cerr << "Error: Failed to accept client connection." << std::endl;
            close(server_socket);
            return 1;
        }
        seed = static_cast<uint32_t>(std::random_device{}());
        game = Game(height, width, 2);
        game.setSeed(seed);
        if (!send_handshake(network_socket, seed, width, height)) {
            std::cerr << "Error: Could not send handshake to client." << std::endl;
            close(network_socket);
            close(server_socket);
            return 1;
        }
        if (!setNonBlocking(network_socket)) {
            std::cerr << "Error: Could not set network socket non-blocking." << std::endl;
            close(network_socket);
            close(server_socket);
            return 1;
        }
    } else if (mode == MODE_ONLINE_CLIENT) {
        uint32_t remote_seed;
        if (!recv_handshake(network_socket, remote_seed, width, height)) {
            std::cerr << "Error: Failed to receive handshake from host." << std::endl;
            close(network_socket);
            return 1;
        }
        gui = create_gui(width, height);
        if (!gui) {
            std::cerr << "Error: Failed to create GUI after handshake." << std::endl;
            close(network_socket);
            return 1;
        }
        if (!setNonBlocking(network_socket)) {
            std::cerr << "Error: Could not set socket to non-blocking mode." << std::endl;
            close(network_socket);
            return 1;
        }
        seed = remote_seed;
        game = Game(height, width, 2);
        game.setSeed(seed);
    } else {
        game = Game(height, width, nbPlayer);
    }

    bool running = true;
    auto last_move = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t> network_buffer;
    bool remote_quit = false;

    while (running) {
        display_gui(gui, game);

        int input = input_gui(gui);
        if (mode == MODE_ONLINE_HOST || mode == MODE_ONLINE_CLIENT) {
            if (input == -1) {
                if (network_socket >= 0) send_quit_message(network_socket);
                running = false;
                break;
            }
            if (mode == MODE_ONLINE_HOST) {
                if (input == UP || input == DOWN || input == LEFT || input == RIGHT) {
                    game.changeDirection(static_cast<Direction>(input));
                    if (!send_input_message(network_socket, input)) {
                        std::cerr << "Error: Failed to send input to client." << std::endl;
                        running = false;
                        break;
                    }
                }
            } else {
                if (input == 110 || input == 111 || input == 112 || input == 113) {
                    Direction direction = NONE;
                    if (input == 110) direction = LEFT;
                    if (input == 111) direction = RIGHT;
                    if (input == 112) direction = UP;
                    if (input == 113) direction = DOWN;
                    game.changeDirection2(direction);
                    if (!send_input_message(network_socket, input)) {
                        std::cerr << "Error: Failed to send input to host." << std::endl;
                        running = false;
                        break;
                    }
                }
            }

            if (network_socket >= 0) {
                std::vector<int> remote_inputs;
                if (!drain_network(network_socket, network_buffer, remote_inputs, remote_quit)) {
                    running = false;
                    break;
                }
                for (int remote_input : remote_inputs) {
                    if (mode == MODE_ONLINE_HOST) {
                        if (remote_input == 110 || remote_input == 111 || remote_input == 112 || remote_input == 113) {
                            Direction direction = NONE;
                            if (remote_input == 110) direction = LEFT;
                            if (remote_input == 111) direction = RIGHT;
                            if (remote_input == 112) direction = UP;
                            if (remote_input == 113) direction = DOWN;
                            game.changeDirection2(direction);
                        }
                    } else if (mode == MODE_ONLINE_CLIENT) {
                        if (remote_input == UP || remote_input == DOWN || remote_input == LEFT || remote_input == RIGHT) {
                            game.changeDirection(static_cast<Direction>(remote_input));
                        }
                    }
                }
                if (remote_quit) {
                    std::cerr << "Remote player disconnected. Ending game." << std::endl;
                    running = false;
                    break;
                }
            }
        } else {
            if (input == -1) {
                running = false;
            } else if (input == 10 && !load_lib("./lib_sdl3.so", currentLibrary)) {
                running = false;
            } else if (input == UP) {
                game.changeDirection(UP);
            } else if (input == DOWN) {
                game.changeDirection(DOWN);
            } else if (input == LEFT) {
                game.changeDirection(LEFT);
            } else if (input == RIGHT) {
                game.changeDirection(RIGHT);
            } else if (nbPlayer == 2 && input == 110) {
                game.changeDirection2(LEFT);
            } else if (nbPlayer == 2 && input == 111) {
                game.changeDirection2(RIGHT);
            } else if (nbPlayer == 2 && input == 112) {
                game.changeDirection2(UP);
            } else if (nbPlayer == 2 && input == 113) {
                game.changeDirection2(DOWN);
            }
        }

        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_move).count();

        if (elapsed >= TICK_RATE) {
            if (game.moveSnake() == -1) {
                running = false;
            }
            if (game.getNbPlayer() == 2 && game.moveSnake2() == -1) {
                running = false;
            }
            last_move = now;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    if (gui) destroy_gui(gui);
    if (handle) dlclose(handle);
    if (network_socket >= 0) close(network_socket);
    if (server_socket >= 0) close(server_socket);

    return 0;
}
