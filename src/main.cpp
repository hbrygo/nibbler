#include "nibbler.hpp"
#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <cctype>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <random>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

int currentLibrary = SDL3;

typedef void* (*create_t)(int, int, bool);
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

static bool isNumber(const std::string& s)
{
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

static bool send_all(int socket_fd, const void* data, size_t length)
{
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(data);
    while (length > 0) {
        ssize_t sent = send(socket_fd, buffer, length, 0);
        if (sent <= 0) {
            return false;
        }
        buffer += sent;
        length -= static_cast<size_t>(sent);
    }
    return true;
}

static bool recv_all(int socket_fd, void* data, size_t length)
{
    uint8_t* buffer = reinterpret_cast<uint8_t*>(data);
    while (length > 0) {
        ssize_t received = recv(socket_fd, buffer, length, MSG_WAITALL);
        if (received <= 0) {
            return false;
        }
        buffer += received;
        length -= static_cast<size_t>(received);
    }
    return true;
}

static bool setNonBlocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) >= 0;
}

static int create_server_socket(int port, int& actual_port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<uint16_t>(port));

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

static int create_client_socket(const char* host, int port)
{
    struct addrinfo hints{};
    struct addrinfo* res = nullptr;
    struct addrinfo* p = nullptr;
    int client_fd = -1;

    char port_str[6] = {0};
    std::snprintf(port_str, sizeof(port_str), "%d", port);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        return -1;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (client_fd < 0) {
            continue;
        }
        if (connect(client_fd, p->ai_addr, p->ai_addrlen) < 0) {
            close(client_fd);
            client_fd = -1;
            continue;
        }
        break;
    }

    freeaddrinfo(res);
    return client_fd;
}

static bool send_handshake(int socket_fd, uint32_t seed, int width, int height)
{
    uint8_t buffer[9];
    buffer[0] = MSG_HANDSHAKE;

    uint32_t net_seed = htonl(seed);
    std::memcpy(&buffer[1], &net_seed, sizeof(net_seed));

    uint16_t net_height = htons(static_cast<uint16_t>(height));
    uint16_t net_width = htons(static_cast<uint16_t>(width));
    std::memcpy(&buffer[5], &net_height, sizeof(net_height));
    std::memcpy(&buffer[7], &net_width, sizeof(net_width));

    return send_all(socket_fd, buffer, sizeof(buffer));
}

static bool recv_handshake(int socket_fd, uint32_t& seed, int& width, int& height)
{
    uint8_t buffer[9];
    if (!recv_all(socket_fd, buffer, sizeof(buffer))) {
        return false;
    }
    if (buffer[0] != MSG_HANDSHAKE) {
        return false;
    }

    uint32_t net_seed = 0;
    std::memcpy(&net_seed, &buffer[1], sizeof(net_seed));
    seed = ntohl(net_seed);

    uint16_t net_height = 0;
    uint16_t net_width = 0;
    std::memcpy(&net_height, &buffer[5], sizeof(net_height));
    std::memcpy(&net_width, &buffer[7], sizeof(net_width));

    height = ntohs(net_height);
    width = ntohs(net_width);
    return true;
}

static bool send_input_message(int socket_fd, int input_code)
{
    uint8_t buffer[5];
    buffer[0] = MSG_INPUT;
    int32_t payload = htonl(static_cast<int32_t>(input_code));
    std::memcpy(&buffer[1], &payload, sizeof(payload));
    return send_all(socket_fd, buffer, sizeof(buffer));
}

static bool send_quit_message(int socket_fd)
{
    uint8_t buffer[1] = { MSG_QUIT };
    return send_all(socket_fd, buffer, sizeof(buffer));
}

static bool drain_network(int socket_fd, std::vector<uint8_t>& buffer, std::vector<int>& remote_inputs, bool& remote_quit)
{
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
            if (pos + 5 > buffer.size()) {
                break;
            }
            int32_t net_input = 0;
            std::memcpy(&net_input, &buffer[pos + 1], sizeof(net_input));
            remote_inputs.push_back(static_cast<int>(ntohl(net_input)));
            pos += 5;
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
        buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(pos));
    }
    return true;
}

struct GameConfig {
    int width = 10;
    int height = 10;

    std::string library = "sdl3";

    bool michaelMode = false;
    bool despawnApple = false;

    bool multiplayer = false;
    bool online = false;
    bool host = false;

    std::string roomId;
};

int askChoice(const std::string& title, const std::vector<std::string>& options)
{
    int input;

    while (true) {
        std::cout << "\n=== " << title << " ===\n\n";

        for (size_t i = 0; i < options.size(); i++) {
            std::cout << i + 1 << ") " << options[i] << "\n";
        }

        std::cout << "\nReponse: ";
        std::cin >> input;

        if (!std::cin.fail() && input >= 1 && input <= (int)options.size())
            return input;

        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
}

int askInt(const std::string& label, int def)
{
    std::string s;
    std::cout << label << " (defaut " << def << "): ";
    std::getline(std::cin >> std::ws, s);
    return s.empty() ? def : std::stoi(s);
}

std::string askString(const std::string& label)
{
    std::string s;
    std::cout << label << ": ";
    std::getline(std::cin >> std::ws, s);
    return s;
}

std::string generateRoomId()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(20000, 59999);
    return std::to_string(dist(gen));
}

GameConfig runMenu()
{
    GameConfig c;

    int mainChoice = askChoice("GAME MODE", {
        "Multi",
        "Solo",
        "Mode_Michael"
    });

    if (mainChoice == 3) {
        c.michaelMode = true;
        c.library = "sdl3";
        c.width = 17;
        c.height = 32;

        std::cout << "Michael Mode Activated!\n";
        return c;
    }

    if (mainChoice == 2) {
        c.despawnApple = askChoice("Despawn Apple?", {"Yes", "No"}) == 1;
        c.width = askInt("Width", 10);
        c.height = askInt("Height", 10);

        int lib = askChoice("Library", {"sdl3", "sfml", "gl"});
        c.library = (lib == 1 ? "sdl3" : lib == 2 ? "sfml" : "gl");

        return c;
    }

    c.multiplayer = true;

    int multiChoice = askChoice("MULTI MODE", {
        "Local",
        "Online"
    });

    if (multiChoice == 1) {
        c.width = askInt("Width", 10);
        c.height = askInt("Height", 10);

        int lib = askChoice("Library", {"sdl3", "sfml", "gl"});
        c.library = (lib == 1 ? "sdl3" : lib == 2 ? "sfml" : "gl");

        c.online = false;
        return c;
    }

    c.online = true;

    int onlineChoice = askChoice("ONLINE MODE", {
        "Host",
        "Client"
    });

    if (onlineChoice == 1) {

        c.host = true;

        c.width = askInt("Width", 10);
        c.height = askInt("Height", 10);

        int lib = askChoice("Library", {"sdl3", "sfml", "gl"});
        c.library = (lib == 1 ? "sdl3" : lib == 2 ? "sfml" : "gl");

        c.roomId = generateRoomId();

        std::cout << "\n========================\n";
        std::cout << "ROOM CREATED\n";
        std::cout << "ID: " << c.roomId << "\n";
        std::cout << "========================\n";

        return c;
    }

    c.host = false;

    std::cout << "Waiting for room ID...\n";
    c.roomId = askString("Enter room ID");

    return c;
}

int main()
{
    int onAppleSound = 0;
    std::mutex onAppleMutex;
    std::atomic<bool> soundRunning(true);
    std::thread sound_thread;

    GameConfig config = runMenu();

    bool michaelMode = config.michaelMode;
    bool despawnApple = config.despawnApple;

    AppMode mode = MODE_SOLO;
    if (config.multiplayer && config.online) {
        mode = config.host ? MODE_ONLINE_HOST : MODE_ONLINE_CLIENT;
    } else if (config.multiplayer) {
        mode = MODE_LOCAL;
    }

    const char* selected_library = config.library.c_str();

    const char* available[] = {"sdl3", "sfml", "gl"};
    bool valid = false;

    for (auto & i : available)
        if (strcmp(selected_library, i) == 0)
            valid = true;

    if (!valid) {
        std::cerr << "Invalid library\n";
        return 1;
    }

    currentLibrary =
        (selected_library[0] == 's' && selected_library[1] == 'f') ? SFML :
        (selected_library[0] == 's' && selected_library[1] == 'd') ? SDL3 : GL;

    int width = config.width;
    int height = config.height;
    int nbPlayer = (mode == MODE_SOLO) ? 1 : 2;

    int network_socket = -1;
    int server_socket = -1;
    int listen_port = 0;
    uint32_t seed = 0;

    if (mode != MODE_ONLINE_CLIENT && (width <= 9 || height <= 9 || width > 100 || height > 100)) {
        std::cerr << "Invalid size\n";
        return 1;
    }

    if (mode == MODE_ONLINE_CLIENT) {
        if (!isNumber(config.roomId)) {
            std::cerr << "Invalid room ID (expected port)\n";
            return 1;
        }

        int port = std::atoi(config.roomId.c_str());
        if (port <= 0 || port > 65535) {
            std::cerr << "Invalid room ID (expected port)\n";
            return 1;
        }

        network_socket = create_client_socket("127.0.0.1", port);
        if (network_socket < 0) {
            std::cerr << "Could not connect to host on localhost:" << port << "\n";
            return 1;
        }

        uint32_t remote_seed = 0;
        if (!recv_handshake(network_socket, remote_seed, width, height)) {
            std::cerr << "Failed to receive handshake from host\n";
            close(network_socket);
            return 1;
        }

        if (width <= 9 || height <= 9 || width > 100 || height > 100) {
            std::cerr << "Invalid size from host\n";
            close(network_socket);
            return 1;
        }

        seed = remote_seed;

        if (!setNonBlocking(network_socket)) {
            std::cerr << "Could not set socket non-blocking\n";
            close(network_socket);
            return 1;
        }
    }

    void* gui = nullptr;
    void* handle = nullptr;

    create_t create_gui = nullptr;
    destroy_t destroy_gui = nullptr;
    display_t display_gui = nullptr;
    input_t input_gui = nullptr;

    auto lib_path = [](int mode) {
        if (mode == SFML) return "./lib_sfml.so";
        if (mode == SDL3) return "./lib_sdl3.so";
        return "./lib_gl.so";
    };

    auto load_lib = [&](const char* lib, int mode) -> bool {

        if (handle) {
            if (gui) {
                destroy_gui(gui);
                gui = nullptr;
            }
            dlclose(handle);
            handle = nullptr;
        }

        handle = dlopen(lib, RTLD_LAZY);
        if (!handle) return false;

        const char* suffix =
            (mode == SFML) ? "sfml" :
            (mode == SDL3) ? "sdl3" : "gl";

        create_gui = (create_t)dlsym(handle, (std::string("create_gui_") + suffix).c_str());
        destroy_gui = (destroy_t)dlsym(handle, (std::string("destroy_gui_") + suffix).c_str());
        display_gui = (display_t)dlsym(handle, (std::string("display_gui_") + suffix).c_str());
        input_gui = (input_t)dlsym(handle, (std::string("input_gui_") + suffix).c_str());

        if (!create_gui || !destroy_gui || !display_gui || !input_gui) {
            dlclose(handle);
            handle = nullptr;
            return false;
        }

        gui = create_gui(width, height, michaelMode);
        if (!gui) {
            dlclose(handle);
            handle = nullptr;
            return false;
        }
        return true;
    };

    if (!load_lib(lib_path(currentLibrary), currentLibrary))
        return 1;

    if (mode == MODE_ONLINE_HOST) {
        if (!isNumber(config.roomId)) {
            std::cerr << "Invalid room ID (expected port)\n";
            if (gui) destroy_gui(gui);
            if (handle) dlclose(handle);
            return 1;
        }

        int requested_port = std::atoi(config.roomId.c_str());
        if (requested_port <= 0 || requested_port > 65535) {
            std::cerr << "Invalid room ID (expected port)\n";
            if (gui) destroy_gui(gui);
            if (handle) dlclose(handle);
            return 1;
        }

        server_socket = create_server_socket(requested_port, listen_port);
        if (server_socket < 0) {
            std::cerr << "Unable to open listening socket on localhost:" << requested_port << "\n";
            if (gui) destroy_gui(gui);
            if (handle) dlclose(handle);
            return 1;
        }

        std::cerr << "Online host listening on localhost:" << listen_port << "\n";
        std::cerr << "Waiting for the other player to connect...\n";

        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        network_socket = accept(server_socket, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
        if (network_socket < 0) {
            std::cerr << "Failed to accept client connection\n";
            close(server_socket);
            if (gui) destroy_gui(gui);
            if (handle) dlclose(handle);
            return 1;
        }

        seed = static_cast<uint32_t>(std::random_device{}());
        if (!send_handshake(network_socket, seed, width, height)) {
            std::cerr << "Could not send handshake to client\n";
            close(network_socket);
            close(server_socket);
            if (gui) destroy_gui(gui);
            if (handle) dlclose(handle);
            return 1;
        }

        if (!setNonBlocking(network_socket)) {
            std::cerr << "Could not set network socket non-blocking\n";
            close(network_socket);
            close(server_socket);
            if (gui) destroy_gui(gui);
            if (handle) dlclose(handle);
            return 1;
        }
    }

    Game game(width, height, nbPlayer, michaelMode, despawnApple);

    bool running = true;
    auto last_move = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t> network_buffer;
    bool remote_quit = false;

    while (running) {

        display_gui(gui, game);

        int input = input_gui(gui);

        if (mode == MODE_ONLINE_HOST || mode == MODE_ONLINE_CLIENT) {
            if (input == -1) {
                if (network_socket >= 0) {
                    send_quit_message(network_socket);
                }
                running = false;
                break;
            }

            if (mode == MODE_ONLINE_HOST) {
                if (input == UP || input == DOWN || input == LEFT || input == RIGHT) {
                    game.changeDirection(static_cast<Direction>(input));
                    if (!send_input_message(network_socket, input)) {
                        std::cerr << "Failed to send input to client\n";
                        running = false;
                        break;
                    }
                }
            } else {
                if (input == P2_LEFT || input == P2_RIGHT || input == P2_UP || input == P2_DOWN) {
                    Direction direction = NONE;
                    if (input == P2_LEFT) direction = LEFT;
                    if (input == P2_RIGHT) direction = RIGHT;
                    if (input == P2_UP) direction = UP;
                    if (input == P2_DOWN) direction = DOWN;

                    game.changeDirection2(direction);
                    if (!send_input_message(network_socket, input)) {
                        std::cerr << "Failed to send input to host\n";
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
                        if (remote_input == P2_LEFT || remote_input == P2_RIGHT || remote_input == P2_UP || remote_input == P2_DOWN) {
                            Direction direction = NONE;
                            if (remote_input == P2_LEFT) direction = LEFT;
                            if (remote_input == P2_RIGHT) direction = RIGHT;
                            if (remote_input == P2_UP) direction = UP;
                            if (remote_input == P2_DOWN) direction = DOWN;
                            game.changeDirection2(direction);
                        }
                    } else {
                        if (remote_input == UP || remote_input == DOWN || remote_input == LEFT || remote_input == RIGHT) {
                            game.changeDirection(static_cast<Direction>(remote_input));
                        }
                    }
                }

                if (remote_quit) {
                    std::cerr << "Remote player disconnected. Ending game.\n";
                    running = false;
                    break;
                }
            }
        } else {
            if (input == -1) running = false;

            else if (input == 10 && !load_lib(lib_path(SDL3), SDL3)) running = false;
            else if (input == 20 && !load_lib(lib_path(SFML), SFML)) running = false;
            else if (!michaelMode && input == 30 && !load_lib(lib_path(GL), GL)) running = false;

            else if (input == UP) game.changeDirection(UP);
            else if (input == DOWN) game.changeDirection(DOWN);
            else if (input == LEFT) game.changeDirection(LEFT);
            else if (input == RIGHT) game.changeDirection(RIGHT);
            else if (mode == MODE_LOCAL && input == P2_UP) game.changeDirection2(UP);
            else if (mode == MODE_LOCAL && input == P2_DOWN) game.changeDirection2(DOWN);
            else if (mode == MODE_LOCAL && input == P2_LEFT) game.changeDirection2(LEFT);
            else if (mode == MODE_LOCAL && input == P2_RIGHT) game.changeDirection2(RIGHT);
        }

        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_move).count();

        if (elapsed >= TICK_RATE) {
            if (game.moveSnake(onAppleSound, onAppleMutex) == -1)
                running = false;
            if (running && game.getNbPlayer() == 2 && game.moveSnake2(onAppleSound, onAppleMutex) == -1)
                running = false;

            last_move = now;
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    soundRunning = false;

    if (sound_thread.joinable())
        sound_thread.join();

    if (gui) destroy_gui(gui);
    if (handle) dlclose(handle);
    if (network_socket >= 0) close(network_socket);
    if (server_socket >= 0) close(server_socket);

    return 0;
}