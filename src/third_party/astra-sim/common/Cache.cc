#include "astra-sim/common/Cache.hh"

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

using namespace AstraSim::Utils;

template <typename K, typename T>
void Cache<K, T>::put(const K& key, const T& value) {
  std::lock_guard<std::mutex> lock(cache_mutex);
  this->writing.store(true);
  if (this->cache.find(key) != this->cache.end()) {
    this->lru.erase(cache[key].second);
    this->lru.push_back(key);
    this->cache[key].second = --this->lru.end();
    this->cache[key].first =
        std::static_pointer_cast<const T>(std::make_shared<T>(value));
  } else {
    if (cache.size() >= capacity) {
      auto lru_node = this->lru.front();
      this->cache.erase(lru_node);
      this->lru.pop_front();
    }
    this->lru.push_back(key);
    this->cache[key] = {
        std::static_pointer_cast<const T>(std::make_shared<T>(value)),
        --this->lru.end()};
  }
  this->writing.store(false);
}

template <typename K, typename T>
bool Cache<K, T>::has(const K& key) const {
  while (this->writing.load()) {
    std::this_thread::yield();
  }
  return this->cache.find(key) != this->cache.end();
}

template <typename K, typename T>
const std::weak_ptr<const T> Cache<K, T>::get(const K& key) const {
  while (this->writing.load()) {
    std::this_thread::yield();
  }
  if (this->cache.find(key) == this->cache.end())
    throw std::runtime_error("Node not found in cache");
  const std::weak_ptr<const T> weak_ptr(this->cache.at(key).first);
  return weak_ptr;
}

template <typename K, typename T>
const std::shared_ptr<const T> Cache<K, T>::get_locked(const K& key) const {
  while (this->writing.load()) {
    std::this_thread::yield();
  }
  if (this->cache.find(key) == this->cache.end())
    throw std::runtime_error("Node not found in cache");
  return this->cache.at(key).first;
}

template <typename K, typename T>
void Cache<K, T>::remove(const K& key) {
  std::lock_guard<std::mutex> lock(cache_mutex);
  this->writing.store(true);
  if (!this->has(key)) {
    return;
  }
  this->lru.erase(cache[key].second);
  this->cache.erase(key);
  this->writing.store(false);
}
