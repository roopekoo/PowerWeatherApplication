#ifndef DATALINESMODEL_HH
#define DATALINESMODEL_HH

#include "apitypes.hh"

#include <QTimer>
#include <unordered_set>

namespace pwa {

using DataLineId = std::size_t;

/*!
 * \brief The DataLineWrapper struct holds extra information, that is useful
 * only for model. Only DataLine property is shared with users of
 * DataLinesModel.
 */
struct DataLineWrapper {
	DataLineId id = 0;
	QString name = "";
	bool enabled = false;
	std::shared_ptr<DataLine> dataLine = nullptr;
};

/*!
 * \brief DataLinesModel holds business logic of getting data lines from the web
 * and maintaining them. Provides an interface with easy to change settings
 * that trigger automatic data fetching when it is needed. Has signals that
 * notify state changes to subscribers. Real time updates are always on and
 * handled automatically.
 */
class DataLinesModel: public QObject {
	Q_OBJECT
  public:
	/*!
	 * \brief DataLinesModel costructor creates initial datalines and sets up
	 * timers.
	 * \param parent
	 */
	explicit DataLinesModel(QObject* parent);

	DataLinesModel(const DataLinesModel&) = delete;
	DataLinesModel(DataLinesModel&&) = delete;
	DataLinesModel& operator=(const DataLinesModel&) = delete;
	DataLinesModel& operator=(DataLinesModel&&) = delete;

	/*!
	 * \brief Returns all data line ids (initial data lines and data lines that
	 * have been imported during this session).
	 * \return All data line ids
	 */
	std::vector<DataLineId> getAllIds() const;

	/*!
	 * \brief Returns initial dataline ids. Initial datalines are those which
	 * are implemented in webAPI and fetched from the internet. Should only be
	 * used for initialization.
	 * \return Initial data line ids.
	 */
	std::vector<DataLineId> getInitialDataIds() const;

	/*!
	 * \brief Gets display name for DataLine with id.
	 * \param id
	 * \return Name.
	 */
	QString getDataName(DataLineId id) const;

	/*!
	 * \brief Gets DataLine for this id.
	 * \param id
	 * \return Data line.
	 */
	std::shared_ptr<DataLine> getDataLine(DataLineId id) const;

	/*!
	 * \brief Sets current time span for data fetching. Updates all currently
	 * enabled DataLines if needed.
	 * \param timeSpan
	 */
	void setTimeSpan(const TimeSpan& timeSpan);

	/*!
	 * \brief Getter for TimeSpan property.
	 * \return TimeSpan
	 */
	TimeSpan getTimeSpan() const;

	/*!
	 * \brief Sets current location for data fetching. Updates all currently
	 * enabled DataLines if needed.
	 * \param location
	 */
	void setLocation(const QString& location);

	/*!
	 * \brief Getter for location property.
	 * \return location
	 */
	QString getLocation() const;

	/*!
	 * \brief Sets enabled status for DataLine with id. Updates the data line
	 * if needed.
	 * \param id
	 * \param enabled Target status.
	 */
	void setDataEnabled(DataLineId id, bool enabled);

	/*!
	 * \brief Returns current enabled status of DataLine with id.
	 * \param id
	 * \return True if DataLine with id is currently enabled (ticked and
	 * fetched). False otherwise.
	 */
	bool isDataEnabled(DataLineId id);

	/*!
	 * \brief Add new DataLine. Name is generated automatically as "Imported N:
	 * ...".
	 * \param dataLine DataLine to add. std::move recommended because it
	 * might be large.
	 * \param name
	 * \return Id of added data line.
	 */
	DataLineId addDataLine(const DataLine& dataLine);

	/*!
	 * \brief Get full unique name for id that is persistent between software
	 * versions. Useful for referring to ids between sessions.
	 * \param id
	 * \pre Id must be an initial.
	 * \return Persistent name.
	 */
	QString idToPersistentName(DataLineId id);

	/*!
	 * \brief Inverse of idToPersistentName.
	 * \param name
	 * \param outId
	 * \return True if id exists for this name, false otherwise.
	 */
	bool persistentNameToId(const QString& name, DataLineId& outId);

  signals:
	/*!
	 * \brief Is fired whenever new DataLine has been fetched or enabled.
	 * \param id
	 */
	void dataLineChanged(DataLineId id);

	/*!
	 * \brief Is fired whenever a DataLine is disabled. May get called even if
	 * this DataLine was already disabled. (getDataLine can return nullptr).
	 * \param id
	 */
	void dataLineDisabled(DataLineId id);

	/*!
	 * \brief Is fired when new real time DataPoints have been fetched.
	 * onDataLineChanged does not get called with these dataPoints.
	 * \param newDataPoints
	 * \param id
	 */
	void newRealTimeData(std::vector<DataPoint> newDataPoints, DataLineId id);

	/*!
	 * \brief Is fired when fetch fails because of invalid location.
	 * \param location
	 */
	void invalidLocation(const QString& location);

  private:
	// Updates internal state, gets called after settings are changed.
	void updateState();

	// Updates state of a single imported DataLineWrapper
	void updateImportedWrapper(DataLineWrapper& wrapper, bool wantedEnabled);

	// Updates state of a single initial DataLineWrapper
	void updateInitialWrapper(DataLineWrapper& wrapper, bool wantedEnabled);

	// Tries to fetch new real time data for the last few minutes for enabled
	// DataLines if they are enabled.
	void realTimeUpdate();

	// Is called when webAPI is done with fetch request.
	void onReceiveFetch(const FetchRequest& origReq, const FetchResult& result,
			DataLineId id);

	void onReceiveRealTimeFetch(const FetchRequest& origReq,
			const FetchResult& result, DataLineId id);

	// Get supported data type and provider combinations from webAPI
	std::vector<std::pair<Provider, DataType>> getInitialDataTypes();

	// Holds results from getInitialDataTypes
	const std::vector<std::pair<Provider, DataType>> initialDataTypes_;

	// Holds all DataLines wrapped into a struct with more useful info like id,
	// enabled status and name.
	std::vector<DataLineWrapper> dataLineWrappers_{};

	// Used to represent user check boxes. User may "want" data line to be
	// enabled, but webAPI is still fetching the data, so it's not yet truly
	// enabled. DataLineWrappers hold the definitive enabled status.
	std::vector<bool> wantedDataLineEnabled_{};

	// Current TimeSpan used for fetching data
	TimeSpan timeSpan_{};
	bool timeSpanChanged = false;

	// Current location used for fetching data.
	QString location_{};
	bool locationChanged = false;

	// Holds old requests to compare new requests to.
	// Prevents fetching the same data again.
	std::unordered_map<DataLineId, FetchRequest> requestCache_{};

	// Used to wait until current event queue has been processed.
	// This way all setTimeSapn(), setLocation() and setDataEnabled()
	// can be called back to back without redundant state changes.
	QTimer stateUpdateTimer_{};

	// Used to periodically refetch all data for the last few minuts to update
	// it in real time.
	QTimer realTimeUpdateTimer_{};
};

}  // namespace pwa
#endif  // DATALINESMODEL_HH
