#include "settings.h"

#include <esp_log.h>
#include <nvs_flash.h>
#include <cstring>

#define TAG "Settings"

Server_Urls Read_server_url()
{
    ESP_LOGI(TAG, "read server url");
    nvs_handle_t nvs_handle;
    auto ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        // The namespace doesn't exist, just return default values
        ESP_LOGW(TAG, "NVS namespace %s doesn't exist", NVS_NAMESPACE);
        nvs_close(nvs_handle);
        return {"http://coze.nbee.net/xiaozhi/ota/", "ws://coze.nbee.net/xiaozhi/v1/"};
    }

    Server_Urls serverUrls = {};

    size_t length = sizeof(serverUrls.ota_url);
    if (nvs_get_str(nvs_handle, "ota_url", serverUrls.ota_url, &length) != ESP_OK) {
        ESP_LOGE(TAG, "GET ota_url FAIL");
        // If failed, use default OTA URL
        strncpy(serverUrls.ota_url, "http://coze.nbee.net/xiaozhi/ota/", sizeof(serverUrls.ota_url));
    }

    length = sizeof(serverUrls.websocket_url);
    if (nvs_get_str(nvs_handle, "websocket_url", serverUrls.websocket_url, &length) != ESP_OK) {
        ESP_LOGE(TAG, "GET websocket_url FAIL");
        // If failed, use default websocket URL
        strncpy(serverUrls.websocket_url, "ws://coze.nbee.net/xiaozhi/v1/", sizeof(serverUrls.websocket_url));
    }

    nvs_close(nvs_handle);
    return serverUrls;
}


Settings::Settings(const std::string& ns, bool read_write) : ns_(ns), read_write_(read_write) {
    nvs_open(ns.c_str(), read_write_ ? NVS_READWRITE : NVS_READONLY, &nvs_handle_);
}

Settings::~Settings() {
    if (nvs_handle_ != 0) {
        if (read_write_ && dirty_) {
            ESP_ERROR_CHECK(nvs_commit(nvs_handle_));
        }
        nvs_close(nvs_handle_);
    }
}

std::string Settings::GetString(const std::string& key, const std::string& default_value) {
    if (nvs_handle_ == 0) {
        return default_value;
    }

    size_t length = 0;
    if (nvs_get_str(nvs_handle_, key.c_str(), nullptr, &length) != ESP_OK) {
        return default_value;
    }

    std::string value;
    value.resize(length);
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle_, key.c_str(), value.data(), &length));
    while (!value.empty() && value.back() == '\0') {
        value.pop_back();
    }
    return value;
}




void Settings::SetString(const std::string& key, const std::string& value) {
    if (read_write_) {
        ESP_ERROR_CHECK(nvs_set_str(nvs_handle_, key.c_str(), value.c_str()));
        dirty_ = true;
    } else {
        ESP_LOGW(TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

int32_t Settings::GetInt(const std::string& key, int32_t default_value) {
    if (nvs_handle_ == 0) {
        return default_value;
    }

    int32_t value;
    if (nvs_get_i32(nvs_handle_, key.c_str(), &value) != ESP_OK) {
        return default_value;
    }
    return value;
}

void Settings::SetInt(const std::string& key, int32_t value) {
    if (read_write_) {
        ESP_ERROR_CHECK(nvs_set_i32(nvs_handle_, key.c_str(), value));
        dirty_ = true;
    } else {
        ESP_LOGW(TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

void Settings::EraseKey(const std::string& key) {
    if (read_write_) {
        auto ret = nvs_erase_key(nvs_handle_, key.c_str());
        if (ret != ESP_ERR_NVS_NOT_FOUND) {
            ESP_ERROR_CHECK(ret);
        }
    } else {
        ESP_LOGW(TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

void Settings::EraseAll() {
    if (read_write_) {
        ESP_ERROR_CHECK(nvs_erase_all(nvs_handle_));
    } else {
        ESP_LOGW(TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}
