/*!
	\note The DataLineSaver, Preference
	and ChartPresenter classes probably
	need large scale refactoring.
*/

#ifndef DATALINE_SAVER
#define DATALINE_SAVER

#include "apitypes.hh"
#include "jsonmanager.hh"

namespace pwa {

/*!
	\brief DataLineSaver is a tool
	for saving and loading struct DataLine
*/
/*	Maybe replace this class by
	making DataLine a class.
	A possible problem of the current implementation:
	DataLineSaver is highly dependent on DataLine.
*/
class DataLineSaver {
  public:
	/*!
		\brief Construct DataLineSaver associated
		with saveFilePath.
		\param saveFilePath the path to the file
		to use for writing/reading the saved data.
		\note Be careful not to overwrite others'
		files unknowingly.
	*/
	explicit DataLineSaver(const QString& saveFilePath);

	DataLineSaver(const DataLineSaver&) = delete;
	DataLineSaver(DataLineSaver&&) = delete;
	DataLineSaver& operator=(const DataLineSaver&) = delete;
	DataLineSaver& operator=(DataLineSaver&&) = delete;

	virtual ~DataLineSaver() = default;

	/*!
		\brief Restore the first DataLine from
		the saved JSON file.
		\return true if successful, false otherwise
		\see write
	*/
	bool readTo(DataLine& data) const;

	/*!
		\brief Restore DataLines from the saved JSON file.
		\return true if successful, false otherwise
		\note If the JSON was written with
		bool write(const DataLine& data) const, this doesn't work.
		\see write
	*/
	bool readTo(std::vector<DataLine>& data) const;

	/*!
		\brief Store DataLine to the JSON file.
		\return true if successful, false otherwise
		\see readTo(DataLine& data)
	*/
	bool write(const DataLine& data) const;

	/*!
		\brief Store many DataLines to the single JSON file.
		\return true if successful, false otherwise
		\see readTo
	*/
	bool write(const std::vector<std::shared_ptr<DataLine>>& data) const;

  private:
	JsonManager dataLineManager_;

	/*!
		\brief QJsonObject to the corresponding DataLine
	*/
	inline bool restore(
			DataLine& dataLine, const QJsonObject& jsonDataLine) const;
	//	Helper functions.
	inline void restoreSingleValues(
			DataLine& dataLine, const QJsonObject& jsonDataLine) const;
	inline void restoreTimeSpan(
			DataLine& dataLine, const QJsonObject& jsonDataLine) const;
	inline void restoreDataPoints(
			DataLine& dataLine, const QJsonObject& jsonDataLine) const;
	inline QDateTime dataPointX(const QJsonValue& dataPoint) const;
	inline double dataPointY(const QJsonValue& dataPoint) const;

	//	Move these into a separate converter class?
	/*!
		\brief DataLine to the corresponding QJsonObject
	*/
	inline QJsonObject toQJsonObject(const DataLine& data) const;
	/*!
		\brief Many DataLines to the corresponding QJsonArray
	*/
	inline QJsonArray toQJsonArray(
			const std::vector<std::shared_ptr<DataLine>>& data) const;
	//	Helper functions.
	inline QJsonObject toQJsonObject(const TimeSpan& timeSpan) const;
	inline QJsonArray toQJsonArray(
			const std::vector<DataPoint>& dataPoints) const;
	inline QJsonObject toQJsonObject(const DataPoint& dataPoint) const;

	//	Move these into a separate validator class?
	/*!
		\brief Validate DataLine JSON.
	*/
	inline bool isValidDataLineJson(const QJsonObject& json) const;
	/*
		Helper functions.
	*/
	inline bool isTimeSpanValid(const QJsonObject& json) const;
	inline bool isTimeSpanContentsValid(QJsonObject&& timeSpan) const;
	inline bool isDataPointsValid(const QJsonObject& json) const;
	inline bool isDataPointsContentsValid(QJsonArray&& dataPoints) const;
	inline bool isDataPointValid(const QJsonObject& dataPoint) const;
};

}  // namespace pwa
#endif  //	DATALINE_SAVER
