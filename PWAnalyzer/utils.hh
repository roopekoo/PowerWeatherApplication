#ifndef UTILS_H
#define UTILS_H

#include "apitypes.hh"

#include <vector>

namespace pwa {
namespace utils {

	/*!
	 * \brief Averages all DataPoints' y values
	 * Uses getSum function to calculate the whole sum before averaging
	 * \param dataPoints
	 * \return Averaged y value
	 */
	double averageDataPoints(const std::vector<DataPoint>& dataPoints);

	/*!
	 * \brief Create a string of date with timezone appended in iso compliant
	 * format
	 * \param dateTime
	 * \return Created string
	 */
	QString toIsoStringWithTZ(const QDateTime& dateTime);

	QDateTime fromIsoString(const QString& isoString);

	/*!
	 * \brief getSum
	 * Calculate the sum of all dataPoints
	 * \param dataPoints
	 * \return sum of all values of the datapoints
	 */
	double getSum(const std::vector<DataPoint>& dataPoints);

	/*!
	 * \brief averageDoubles
	 * Calculates the average of vector of doubles
	 * \param values
	 * \return average double value
	 */
	double averageDoubles(const std::vector<double>& values);

	/*!
	 * \brief getDataPointsMin
	 * Finds the smallest value in given vector
	 * (smallest as a value, not as an absolute value)
	 * \param dataPoints
	 * \return minimum double value
	 */
	double getDataPointsMin(const std::vector<DataPoint>& dataPoints);

	/*!
	 * \brief getDataPointsMax
	 * Finds the biggest value in given vector
	 * \param dataPoints
	 * \return maximum double value
	 */
	double getDataPointsMax(const std::vector<DataPoint>& dataPoints);

	// Split timeSpan into parts where each part is shorter than
	// daysPerSpan
	std::vector<TimeSpan> splitTimeSpan(
			const TimeSpan& timeSpan, int daysPerSpan);

	/*!
	 * \brief findNewDataPoints
	 * Returns a copy of the datapoint candidates that have x values higher than
	 * the original datapoints
	 * \param original
	 * \param newCandidates \return Copy
	 * of the new DataPoints
	 */
	std::vector<DataPoint> findNewDataPoints(
			const std::vector<DataPoint>& original,
			const std::vector<DataPoint>& newCandidates);

}  // namespace utils
}  // namespace pwa

#endif  // UTILS_H
