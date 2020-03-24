#pragma once

#include <algorithm>
#include <any>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <unordered_map>

#if COLLECT_METRICS
    #define METRICS_RECORD_BLOCK(metric_name)                 \
	    mtr::collector UNIQUE_NAME(__cOlLeCtOr)((metric_name));
    
    #define UNIQUE_NUM __LINE__
    #define CAT(X, Y) CAT_IMP(X, Y)
    #define CAT_IMP(x, Y) X##Y
    #define UNIQUE_NAME(X) CAT(X, UNIQUE_NUM)
#else
    #define METRICS_RECORD_BLOCK(metric_name)
#endif

namespace mtr {

constexpr std::size_t MAX_RECORDINGS = 1000;

class block_recording {
public:
	void update(std::chrono::nanoseconds elapsed);

	std::size_t times_entered() const;
	typename std::array<std::chrono::nanoseconds, MAX_RECORDINGS>::const_iterator begin() const;
	typename std::array<std::chrono::nanoseconds, MAX_RECORDINGS>::const_iterator end() const;

private:
	std::uint64_t times_entered_ = 0;
	std::array<std::chrono::nanoseconds, MAX_RECORDINGS> recordings_;
};

class high_resolution_timer {
public:
	explicit high_resolution_timer();

	void restart();
    std::chrono::nanoseconds elapsed() const;

private:
	static std::chrono::high_resolution_clock::time_point take_time_stamp();

private:
	std::chrono::high_resolution_clock::time_point start_time_;
};

class collector {
public:
	explicit collector(std::string metric_name);
	~collector();

private:
	std::string _metric_name;
	high_resolution_timer timer_;
};

class metric_aggregator {
public:
	static metric_aggregator &instance();

	void update_metric(const std::string &name, std::chrono::nanoseconds elapsed);

	std::size_t times_entered(const std::string &name) const;

	template <typename T>
	T min(const std::string &name) const;

	template <typename T>
	T max(const std::string &name) const;

	template <typename T>
	std::pair<T, T> min_max(const std::string &name) const;

	template <typename T>
	T average(const std::string &name) const;

    template <typename T>
    T total(const std::string &name) const;

	metric_aggregator(metric_aggregator const &) = delete;
	void operator=(metric_aggregator const &) = delete;

private:
	explicit metric_aggregator() = default;

private:
	std::unordered_map<std::string, block_recording> metrics_;
};

inline void block_recording::update(std::chrono::nanoseconds elapsed) {
	recordings_[times_entered_ % MAX_RECORDINGS] = elapsed;
	++times_entered_;
}

inline std::size_t block_recording::times_entered() const {
	return times_entered_;
}

inline typename std::array<std::chrono::nanoseconds, MAX_RECORDINGS>::const_iterator
block_recording::begin() const {
	return recordings_.cbegin();
}

inline typename std::array<std::chrono::nanoseconds, MAX_RECORDINGS>::const_iterator
block_recording::end() const {
    return times_entered_ >= MAX_RECORDINGS ? recordings_.cend() : recordings_.cbegin() + times_entered_;
}

inline high_resolution_timer::high_resolution_timer()
    : start_time_(take_time_stamp()) {}

inline void high_resolution_timer::restart() {
	start_time_ = take_time_stamp();
}

inline std::chrono::nanoseconds high_resolution_timer::elapsed() const {
	return std::chrono::duration_cast<std::chrono::nanoseconds>(take_time_stamp() - start_time_);
}

inline std::chrono::high_resolution_clock::time_point high_resolution_timer::take_time_stamp() {
	return std::chrono::high_resolution_clock::now();
}

inline collector::collector(std::string metric_name)
    : _metric_name(std::move(metric_name)), timer_() {}

inline collector::~collector() {
	const std::chrono::nanoseconds elapsed = timer_.elapsed();
	metric_aggregator::instance().update_metric(_metric_name, elapsed);
}

inline metric_aggregator &metric_aggregator::instance() {
	static metric_aggregator instance;
	return instance;
}

inline void metric_aggregator::update_metric(const std::string &name, std::chrono::nanoseconds elapsed) {
	const auto iter = metrics_.find(name);
	if (iter != metrics_.end()) {
		iter->second.update(elapsed);
	} else {
		block_recording new_recording;
		new_recording.update(elapsed);

		metrics_.emplace(name, new_recording);
	}
}

inline std::size_t metric_aggregator::times_entered(const std::string &name) const {
	const auto iter = metrics_.find(name);
	if (iter == metrics_.end()) {
		return 0;
	}

	return iter->second.times_entered();
}

template <typename T>
inline T metric_aggregator::min(const std::string &name) const {
	const auto iter = metrics_.find(name);
	if (iter == metrics_.end()) {
		return T{0};
	}

	return std::chrono::duration_cast<T>(*std::min_element(iter->second.begin(), iter->second.end()));
}

template <typename T>
inline T metric_aggregator::max(const std::string &name) const {
	const auto iter = metrics_.find(name);
	if (iter == metrics_.end()) {
		return T{0};
	}

	return std::chrono::duration_cast<T>(*std::max_element(iter->second.begin(), iter->second.end()));
}

template <typename T>
inline std::pair<T, T> metric_aggregator::min_max(const std::string &name) const {
    return {min<T>(name), max<T>(name)};
}

template <typename T>
inline T metric_aggregator::average(const std::string &name) const {
	const auto iter = metrics_.find(name);
	if (iter == metrics_.end()) {
		return T{0};
	}

	const auto nanoseconds = std::accumulate(iter->second.begin(), iter->second.end(), std::chrono::nanoseconds{0});
    return std::chrono::duration_cast<T>(nanoseconds) / std::distance(iter->second.begin(), iter->second.end());
}

template <typename T>
inline T metric_aggregator::total(const std::string &name) const {
	const auto iter = metrics_.find(name);
	if (iter == metrics_.end()) {
		return T{0};
	}

	const auto nanoseconds =  std::accumulate(iter->second.begin(), iter->second.end(), std::chrono::nanoseconds{0});
    return std::chrono::duration_cast<T>(nanoseconds);
}

} // namespace mtr
