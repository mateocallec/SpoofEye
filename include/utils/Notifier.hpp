/**
 * @file Notifier.hpp
 * @brief Desktop notification utility using libnotify for SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include "Config.hpp"

#include <libnotify/notify.h>
#include <string>

/**
 * @class Notifier
 * @brief Provides desktop notifications via libnotify.
 */
class Notifier {
public:
    /** Notification severity levels */
    enum class Level {
        INFO,       ///< Normal informational message
        WARNING,    ///< Warning message
        CRITICAL    ///< Critical alert message
    };

    /**
     * @brief Construct a Notifier.
     * @param enabled Enable notifications (default: true). If libnotify fails to initialize, notifications will be disabled automatically.
     */
    explicit Notifier(bool enabled = true);

    /** @brief Destructor cleans up libnotify resources if initialized */
    ~Notifier();

    /**
     * @brief Send a desktop notification.
     * @param title Notification title.
     * @param message Notification message body.
     * @param level Notification severity (default: INFO).
     * @param icon Optional icon name (default: "dialog-information").
     * @return true if the notification was successfully sent.
     */
    bool send(const std::string& title,
              const std::string& message,
              Level level = Level::INFO,
              const std::string& icon = "dialog-information") const;

private:
    bool enabled_;                 ///< True if notifications are enabled
    bool libnotify_initialized_;   ///< True if libnotify was successfully initialized

    /**
     * @brief Convert notification level to string for internal use.
     * @param level Notification level
     * @return Corresponding string ("normal" or "critical")
     */
    std::string levelToString(Level level) const;
};
