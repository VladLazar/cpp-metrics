#pragma once

#include <algorithm>
#include <any>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <iostream>

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

class block_recording {
public:
	void update(std::chrono::nanoseconds elapsed);

	std::size_t times_entered() const;
    std::chrono::nanoseconds total() const;
    std::chrono::nanoseconds min() const;
    std::chrono::nanoseconds max() const;

private:
	std::uint64_t times_entered_ = 0;
    std::chrono::nanoseconds total_ = std::chrono::nanoseconds(0);
    std::chrono::nanoseconds min_ = std::chrono::nanoseconds::max();
    std::chrono::nanoseconds max_ = std::chrono::nanoseconds::min();
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

    template <typename T>
    void dump_metrics(const std::string &name, std::ostream &stream) const;

    template <typename T>
    void dump_all(std::ostream &stream) const;

	metric_aggregator(metric_aggregator const &) = delete;
	void operator=(metric_aggregator const &) = delete;

private:
	explicit metric_aggregator() = default;

private:
	std::unordered_map<std::string, block_recording> metrics_;
};

template <typename T>
struct stringify_unit {
private:
    constexpr static auto stringify() {
        if constexpr (std::is_same_v<T, std::chrono::nanoseconds>) {
            return "ns";
        } else if constexpr (std::is_same_v<T, std::chrono::microseconds>) {
            return "us";
        } else if constexpr (std::is_same_v<T, std::chrono::milliseconds>) {
            return "ms";
        } else {
            return "s";
        }
    }

public:
    static constexpr auto value = stringify();
};

inline void block_recording::update(std::chrono::nanoseconds elapsed) {
	++times_entered_;
    total_ += elapsed;
    min_ = std::min(elapsed, min_);
    max_ = std::max(elapsed, max_);
}

inline std::size_t block_recording::times_entered() const {
	return times_entered_;
}

inline std::chrono::nanoseconds block_recording::total() const {
    return total_;
}

inline std::chrono::nanoseconds block_recording::min() const {
    return times_entered_ > 0 ? min_ : std::chrono::nanoseconds(0);
}

inline std::chrono::nanoseconds block_recording::max() const {
    return times_entered_ > 0 ? max_ : std::chrono::nanoseconds(0);
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

	return std::chrono::duration_cast<T>(iter->second.min());
}

template <typename T>
inline T metric_aggregator::max(const std::string &name) const {
	const auto iter = metrics_.find(name);
	if (iter == metrics_.end()) {
		return T{0};
	}

	return std::chrono::duration_cast<T>(iter->second.max());
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

	const auto nanoseconds = iter->second.total();
    return std::chrono::duration_cast<T>(nanoseconds) / iter->second.times_entered();
}

template <typename T>
inline T metric_aggregator::total(const std::string &name) const {
	const auto iter = metrics_.find(name);
	if (iter == metrics_.end()) {
		return T{0};
	}

    return std::chrono::duration_cast<T>(iter->second.total());
}

template <typename T>
void metric_aggregator::dump_metrics(const std::string &name, std::ostream &stream) const {
	const auto iter = metrics_.find(name);
	if (iter == metrics_.end()) {
		return;
	}
    
    /* If the duration provided is 'larger' than the std::chrono::seconds,
     * default to std::chrono::seconds. */
    if (not std::is_same_v<T, std::common_type_t<T, std::chrono::seconds>>) {
        dump_metrics<std::chrono::seconds>(name, stream);
        return;
    }

    constexpr auto unit = stringify_unit<T>::value;

    stream << name << " metrics:" << std::endl;
    stream << "\t" << "Entered: " << times_entered(name) << std::endl;
    stream << "\t" << "Total: " << total<T>(name).count() << unit << std::endl;
    stream << "\t" << "Average: " << average<T>(name).count() << unit << std::endl;
    stream << "\t" << "Min: " << min<T>(name).count() << unit << std::endl;
    stream << "\t" << "Max: " << max<T>(name).count() << unit << std::endl;
}

template <typename T>
void metric_aggregator::dump_all(std::ostream &stream) const {
    for (const auto pair : metrics_) {
        dump_metrics<T>(pair.first, stream);
        stream << std::endl;
    }
}

} // namespace mtr
