#ifndef _TRACEPLOTHANDLER_HPP
#define _TRACEPLOTHANDLER_HPP

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Plot.hpp"
#include "PlotHandlerBase.hpp"
#include "StlinkTraceProbe.hpp"
#include "TraceReader.hpp"
#include "spdlog/spdlog.h"

class TracePlotHandler : public PlotHandlerBase
{
   public:
	typedef struct
	{
		uint32_t coreFrequency = 160000;
		uint32_t tracePrescaler = 10;
		uint32_t maxPoints = 10000;
		uint32_t maxViewportPointsPercent = 10;
		int32_t triggerChannel = -1;
		double triggerLevel = 0.9;
		bool shouldReset = false;
		uint32_t timeout = 2;
	} Settings;

	TracePlotHandler(std::atomic<bool>& done, std::mutex* mtx, spdlog::logger* logger);
	~TracePlotHandler();

	void initPlots();

	Settings getSettings() const;
	void setSettings(const Settings& settings);

	TraceReader::TraceIndicators getTraceIndicators() const;
	std::vector<double> getErrorTimestamps();
	std::vector<double> getDelayed3Timestamps();
	std::string getLastReaderError() const;

	void setTriggerChannel(int32_t triggerChannel);
	int32_t getTriggerChannel() const;

	void setDebugProbe(std::shared_ptr<ITraceProbe> probe);
	ITraceProbe::TraceProbeSettings getProbeSettings() const;
	void setProbeSettings(const ITraceProbe::TraceProbeSettings& settings);

	double getDoubleValue(const Plot& plot, uint32_t value);
	std::map<std::string, std::shared_ptr<Variable>> traceVars;

   private:
	void dataHandler();

   private:
	class MarkerTimestamps
	{
	   public:
		void reset()
		{
			timestamps.clear();
			previousErrors = 0;
		}

		void handle(double time, double oldestTimestamp, uint32_t totalErrors)
		{
			if (previousErrors != totalErrors)
				timestamps.push_back(time);

			while (timestamps.size() && timestamps.front() < oldestTimestamp)
				timestamps.pop_front();
			previousErrors = totalErrors;
		}

		size_t size() const
		{
			return timestamps.size();
		}

		auto getVector()
		{
			return std::vector<double>(timestamps.begin(), timestamps.end());
		}

	   private:
		std::deque<double> timestamps;
		uint32_t previousErrors;
	};

	Settings traceSettings{};
	std::unique_ptr<TraceReader> traceReader;

	MarkerTimestamps errorFrames{};
	MarkerTimestamps delayed3Frames{};

	std::string lastErrorMsg{};

	ITraceProbe::TraceProbeSettings probeSettings;

	bool traceTriggered = false;
	static constexpr uint32_t channels = 10;
	static constexpr size_t maxAllowedViewportErrors = 100;
};

#endif