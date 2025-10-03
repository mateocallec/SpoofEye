/**
 * @file Notifier.cpp
 * @brief Desktop notification utility using libnotify for SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "utils/Notifier.hpp"
#include "utils/Logger.hpp"
#include "constants.hpp"

Notifier::Notifier(bool enabled)
    : enabled_(enabled), libnotify_initialized_(false)
{
    if (enabled_) {
        libnotify_initialized_ = notify_init(SOFTWARE_NAME);
        if (!libnotify_initialized_) {
            Logger::log("Failed to initialize libnotify", Logger::LogType::ERROR);
            enabled_ = false; // Disable notifications if initialization fails
        }
    }
}

Notifier::~Notifier() {
    if (libnotify_initialized_) {
        notify_uninit();
    }
}

bool Notifier::send(const std::string& title,
                    const std::string& message,
                    Level level,
                    const std::string& icon) const
{
    if (!enabled_) {
        Logger::log("Notifications are disabled", Logger::LogType::INFO);
        return false;
    }

    NotifyNotification* notification =
        notify_notification_new(title.c_str(), message.c_str(), icon.empty() ? nullptr : icon.c_str());

    // Set urgency based on level
    NotifyUrgency urgency;
    switch(level) {
        case Level::INFO:     
        case Level::WARNING:  urgency = NOTIFY_URGENCY_NORMAL; break;
        case Level::CRITICAL: urgency = NOTIFY_URGENCY_CRITICAL; break;
        default:              urgency = NOTIFY_URGENCY_NORMAL; break;
    }
    notify_notification_set_urgency(notification, urgency);

    // Show the notification
    GError* error = nullptr;
    gboolean success = notify_notification_show(notification, &error);
    if (!success) {
        Logger::log("Failed to send notification: " + std::string(error ? error->message : "Unknown error"), Logger::LogType::ERROR);
        if (error) g_error_free(error);
        g_object_unref(G_OBJECT(notification));
        return false;
    }

    g_object_unref(G_OBJECT(notification));
    return true;
}

std::string Notifier::levelToString(Level level) const {
    switch(level) {
        case Level::INFO:     
        case Level::WARNING:  return "normal";
        case Level::CRITICAL: return "critical";
        default:              return "normal";
    }
}
