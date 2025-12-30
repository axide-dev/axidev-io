/*
 * Simple example for axidev-io showing basic usage.
 *
 * Build with:
 *   cmake -DAXIDEV_IO_BUILD_EXAMPLES=ON ..
 *   cmake --build .
 *
 * Run:
 *   ./axidev-io-example --help
 *
 * Note: Some functionality (global listening, text injection) may require
 * platform permissions (Accessibility / Input Monitoring on macOS, /dev/uinput
 * access on Linux, etc.)
 */

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <axidev-io/keyboard/listener.hpp>
#include <axidev-io/keyboard/sender.hpp>
#include <axidev-io/log.hpp>

int main(int argc, char **argv) {
  using namespace std::chrono_literals;

  axidev::io::keyboard::Sender sender;
  auto caps = sender.capabilities();
  AXIDEV_IO_LOG_INFO(
      "example: sender constructed; type=%d canInjectKeys=%d canInjectText=%d",
      static_cast<int>(sender.type()), static_cast<int>(caps.canInjectKeys),
      static_cast<int>(caps.canInjectText));

  std::cout << "axidev-io example\n";
  AXIDEV_IO_LOG_INFO("example: startup argc=%d", argc);
  std::cout << "  sender type (raw): " << static_cast<int>(sender.type())
            << "\n";
  std::cout << "  capabilities:\n";
  std::cout << "    canInjectKeys:   " << (caps.canInjectKeys ? "yes" : "no")
            << "\n";
  std::cout << "    canInjectText:   " << (caps.canInjectText ? "yes" : "no")
            << "\n";
  std::cout << "    canSimulateHID:  " << (caps.canSimulateHID ? "yes" : "no")
            << "\n";
  std::cout << "    supportsKeyRepeat: "
            << (caps.supportsKeyRepeat ? "yes" : "no") << "\n\n";

  if (argc <= 1) {
    std::cout
        << "Usage:\n"
        << "  --type \"text\"    : inject text (if supported by backend)\n"
        << "  --tap KEYNAME     : tap the named key (e.g., A, Enter, F1)\n"
        << "  --listen N        : listen for global key events for N seconds\n"
        << "  --help            : show this text\n";
    return 0;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help") {
      // Already printed basic usage above
      continue;

    } else if (arg == "--type") {
      if (i + 1 >= argc) {
        std::cerr << "--type requires an argument\n";
        return 1;
      }
      std::string text = argv[++i];
      AXIDEV_IO_LOG_INFO("example: attempting to type: \"%s\"", text.c_str());
      if (!caps.canInjectText) {
        std::cerr << "Backend cannot inject arbitrary text on this "
                     "platform/back-end\n";
      } else {
        std::cout << "Attempting to type: \"" << text << "\"\n";
        bool ok = sender.typeText(text);
        AXIDEV_IO_LOG_INFO("example: typeText result=%u",
                         static_cast<unsigned>(ok));
        std::cout << (ok ? "-> Success\n" : "-> Failed\n");
      }

    } else if (arg == "--tap") {
      if (i + 1 >= argc) {
        std::cerr << "--tap requires a key name (e.g., A, Enter, F1)\n";
        return 1;
      }
      std::string keyName = argv[++i];
      axidev::io::keyboard::Key k = axidev::io::keyboard::stringToKey(keyName);
      if (k == axidev::io::keyboard::Key::Unknown) {
        std::cerr << "Unknown key: " << keyName << "\n";
        continue;
      }
      if (!caps.canInjectKeys) {
        std::cerr << "Sender cannot inject physical keys on this platform\n";
        continue;
      }
      std::cout << "Tapping key: " << axidev::io::keyboard::keyToString(k) << "\n";
      AXIDEV_IO_LOG_INFO("example: tapping key=%s (%s)", keyName.c_str(),
                       axidev::io::keyboard::keyToString(k).c_str());
      bool ok = sender.tap(k);
      AXIDEV_IO_LOG_INFO("example: tap result=%u", static_cast<unsigned>(ok));
      std::cout << (ok ? "-> Success\n" : "-> Failed\n");

    } else if (arg == "--listen") {
      if (i + 1 >= argc) {
        std::cerr << "--listen requires a duration in seconds\n";
        return 1;
      }
      int seconds = std::stoi(argv[++i]);
      AXIDEV_IO_LOG_INFO("example: starting listener for %d seconds", seconds);
      axidev::io::keyboard::Listener listener;
      bool started = listener.start([](char32_t codepoint, axidev::io::keyboard::Key key,
                                       axidev::io::keyboard::Modifier mods, bool pressed) {
        std::cout << (pressed ? "[press]  " : "[release] ")
                  << "Key=" << axidev::io::keyboard::keyToString(key)
                  << " CP=" << static_cast<unsigned>(codepoint) << " Mods=0x"
                  << std::hex << static_cast<int>(static_cast<uint8_t>(mods))
                  << std::dec << "\n";
        AXIDEV_IO_LOG_DEBUG("example: listener %s key=%s cp=%u mods=0x%02x",
                          pressed ? "press" : "release",
                          axidev::io::keyboard::keyToString(key).c_str(),
                          static_cast<unsigned>(codepoint),
                          static_cast<int>(static_cast<uint8_t>(mods)));
      });
      if (!started) {
        AXIDEV_IO_LOG_ERROR("example: listener failed to start");
        std::cerr
            << "Listener failed to start (permissions / platform support?)\n";
      } else {
        AXIDEV_IO_LOG_INFO("example: listener started");
        std::cout << "Listening for " << seconds << " seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        listener.stop();
        AXIDEV_IO_LOG_INFO("example: listener stopped");
        std::cout << "Stopped listening\n";
      }

    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      return 1;
    }
  }

  return 0;
}
