#ifndef _RENDER_MANAGER_TEMPLATE_HPP_
#define _RENDER_MANAGER_TEMPLATE_HPP_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace eng
{

template <typename T>
class TResourceManager
{
public:
    void Add(const std::string& key, std::shared_ptr<T>&&obj)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        objects_[key] = std::move(obj);
    }

    std::shared_ptr<T> Get(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto& find_pair = objects_.find(key);

        if (find_pair == objects_.end()) {
            //use the key as the filename of the resource here
            T *loaded_obj = Load(key.c_str());
            if (loaded_obj) {
                objects_[key] = std::shared_ptr<T>(loaded_obj);
                return objects_[key];
            }
            return nullptr;
        }

        return find_pair->second;
    }

    void ReapUnused()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& obj: objects_) {
            if (!obj.second.unique()) { continue; }
            objects_.erase(obj.first);
        }
    }

protected:
    std::unordered_map<std::string, std::shared_ptr<T>> objects_;

    virtual T *Load(const char*) = 0;
    std::mutex mutex_;
};

}

#endif