/**
 * @file test_integration_sender.cpp
 * @brief Comprehensive integration tests for axidev-io Sender.
 *
 * This file exercises the real OS backend by injecting events and capturing
 * them via STDIN. Ensure the terminal has focus before starting.
 */

#include <algorithm>
#include <axidev-io/core.hpp>
#include <axidev-io/keyboard/sender.hpp>
#include <axidev-io/log.hpp>
#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>

using namespace axidev::io::keyboard;
using namespace std::chrono_literals;

class SenderIntegrationTest : public ::testing::Test {
protected:
  Sender sender;
  Capabilities caps;

  void SetUp() override {
    caps = sender.capabilities();
    AXIDEV_IO_LOG_INFO("Sender Integration Suite: starting integration tests");

    std::cout << "\n====================================================\n"
              << "CORE INTEGRATION TESTS\n"
              << "Target: "
              << (caps.canInjectKeys ? "Hardware/OS Injection" : "Simulated")
              << "\n"
              << "====================================================\n"
              << "IMPORTANT: Keep this terminal window focused!\n"
              << "Press [ENTER] to begin the sequence..." << std::endl;

    std::string start_buffer;
    std::getline(std::cin, start_buffer);
  }
};

TEST_F(SenderIntegrationTest, LayoutMappingZWNumbers) {
  AXIDEV_IO_LOG_INFO("Integration test: Layout Mapping (Z/W & Numbers)");
  std::cout << "[RUNNING] Verifying character mapping (Z, W, 1)..."
            << std::endl;

  auto task = std::async(std::launch::async, [this]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    AXIDEV_IO_LOG_DEBUG("Integration test: sending taps Z W Num1 Enter");
    // Testing Z and W helps identify AZERTY vs QWERTY confusion.
    // Testing Num1 checks if we produce '1' or the shifted symbol.
    sender.tap(Key::Z);
    sender.tap(Key::W);
    sender.tap(Key::Num1);
    sender.tap(Key::Enter);
    return true;
  });

  std::string received;
  std::getline(std::cin, received);
  task.get();
  AXIDEV_IO_LOG_INFO("Integration test: received sequence: %s",
                     received.c_str());

  std::transform(received.begin(), received.end(), received.begin(), ::tolower);

  // We expect these characters to appear regardless of physical layout
  EXPECT_NE(received.find('z'), std::string::npos)
      << "Received sequence: " << received;
  EXPECT_NE(received.find('w'), std::string::npos)
      << "Received sequence: " << received;
  EXPECT_NE(received.find('1'), std::string::npos)
      << "Received sequence: " << received;
}

TEST_F(SenderIntegrationTest, ModifiersAndShiftState) {
  AXIDEV_IO_LOG_INFO("Integration test: Modifiers & Shift State");
  std::cout << "[RUNNING] Verifying Shift modifier (producing 'HELLO')..."
            << std::endl;

  auto task = std::async(std::launch::async, [this]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    AXIDEV_IO_LOG_DEBUG("Integration test: holding shift and sending HELLO");

    sender.holdModifier(Modifier::Shift);
    sender.tap(Key::H);
    sender.tap(Key::E);
    sender.tap(Key::L);
    sender.tap(Key::L);
    sender.tap(Key::O);
    sender.releaseModifier(Modifier::Shift);

    sender.tap(Key::Enter);
    return true;
  });

  std::string received;
  std::getline(std::cin, received);
  task.get();
  AXIDEV_IO_LOG_INFO("Integration test: Received string: %s", received.c_str());

  EXPECT_NE(received.find("HELLO"), std::string::npos)
      << "Received string: " << received;
}

TEST_F(SenderIntegrationTest, LongPressAndRepeat) {
  if (!caps.supportsKeyRepeat) {
    GTEST_SKIP() << "Key repeat not supported on this platform.";
  }

  AXIDEV_IO_LOG_INFO("Integration test: Long Press & Repeat");
  std::cout << "[RUNNING] Testing key repeat (Holding 'X')..." << std::endl;

  auto task = std::async(std::launch::async, [this]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    AXIDEV_IO_LOG_DEBUG("Integration test: keyDown(Key::X)");

    sender.keyDown(Key::X);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(1500)); // Hold long enough for OS repeat
    sender.keyUp(Key::X);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sender.tap(Key::Enter);
    return true;
  });

  std::string received;
  std::getline(std::cin, received);
  task.get();

  size_t count = std::count(received.begin(), received.end(), 'x') +
                 std::count(received.begin(), received.end(), 'X');
  AXIDEV_IO_LOG_INFO("Integration test: Repeat count=%zu", count);

  // Usually, 1.5s should produce at least 5-10 chars depending on OS
  // settings.
  EXPECT_GE(count, 3u) << "Repeat count: " << count;
}
