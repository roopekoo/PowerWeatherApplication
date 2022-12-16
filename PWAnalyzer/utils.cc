#include "utils.hh"

namespace pwa {
namespace utils {
	double averageDataPoints(const std::vector<DataPoint>& dataPoints) {
		double sumY = getSum(dataPoints);
		return sumY / dataPoints.size();
	}

	QString toIsoStringWithTZ(const QDateTime& dateTime) {
		return dateTime.toOffsetFromUtc(dateTime.offsetFromUtc())
				.toString(Qt::ISODate);
	}

	QDateTime fromIsoString(const QString& isoString) {
		return QDateTime::fromString(isoString, Qt::ISODate).toLocalTime();
	}

	double getSum(const std::vector<DataPoint>& dataPoints) {
		if (dataPoints.empty()) {
			return 0;
		}

		double sumY = 0;
		for (const DataPoint& point : dataPoints) {
			sumY += point.y;
		}
		return sumY;
	}

	double averageDoubles(const std::vector<double>& values) {
		double sum = 0;
		for (auto& value : values) {
			sum += value;
		}
		return sum / values.size();
	}

	double getDataPointsMin(const std::vector<DataPoint>& dataPoints) {
		bool firstValue = false;
		double minValue = 0;
		for (auto& value : dataPoints) {
			// Set maxValue on the first round
			if (!firstValue) {
				minValue = value.y;
				firstValue = true;
			} else if (minValue > value.y) {
				minValue = value.y;
			}
		}
		return minValue;
	}

	double getDataPointsMax(const std::vector<DataPoint>& dataPoints) {
		bool firstValue = false;
		double maxValue = 0;
		for (auto& value : dataPoints) {
			// Set maxValue on the first round
			if (!firstValue) {
				maxValue = value.y;
				firstValue = true;
			} else if (maxValue < value.y) {
				maxValue = value.y;
			}
		}
		return maxValue;
	}

	std::vector<TimeSpan> splitTimeSpan(
			const TimeSpan& timeSpan, int daysPerSpan) {
		Q_ASSERT(daysPerSpan > 0);

		std::vector<TimeSpan> parts{
				TimeSpan{timeSpan.start, timeSpan.start.addDays(daysPerSpan)}};

		while (parts.back().end < timeSpan.end) {
			parts.push_back(TimeSpan{
					parts.back().end, parts.back().end.addDays(daysPerSpan)});
		}

		if (parts.back().end > timeSpan.end) {
			parts.back().end = timeSpan.end;
		}

		return parts;
	}

	std::vector<DataPoint> findNewDataPoints(
			const std::vector<DataPoint>& original,
			const std::vector<DataPoint>& newCandidates) {
		if (original.size() == 0) {
			return newCandidates;
		}

		for (auto it = newCandidates.cbegin(); it != newCandidates.cend();
				++it) {
			if ((*it).x > original.back().x) {
				return std::vector<DataPoint>(it, newCandidates.cend());
			}
		}
		return std::vector<DataPoint>{};
	}
}  // namespace utils
}  // namespace pwa
