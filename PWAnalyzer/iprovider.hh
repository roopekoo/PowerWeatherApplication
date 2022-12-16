#ifndef IPROVIDER_H
#define IPROVIDER_H

#include "apitypes.hh"

#include <QNetworkReply>
#include <QNetworkRequest>

namespace pwa {

/*!
 * \brief IProvider defines an interface for all WebAPI providers. Their main
 * jobs are request creation and response parsing. These are done in a way that
 * fits their respective internet api endpoints.
 */
class IProvider {
  public:
	/*!
	 * \brief Returns maximum days for timeSpan which can be fetched in a single
	 * request.
	 * \param fetchRequest Type of request to check.
	 * \return Supported days.
	 */
	virtual int getSupportedDaysPerRequest(
			const FetchRequest& fetchRequest) const = 0;

	/*!
	 * \brief Creates a QNetworkRequest based on information in FetchRequest.
	 * Basically converts our specialized request representation into a generic
	 * qt representation.
	 * \param fetchRequest
	 * \return New QNetworkRequest.
	 */
	virtual QNetworkRequest createQtRequest(
			const FetchRequest& fetchRequest) const = 0;

	/*!
	 * \brief Parses data contained within QNetworkReply and creates our
	 * specialized class FetchResult based on that.
	 * \param response Response object returned by QNetworkAccessManager.
	 * \param originalRequest Original request that was used to fetch this data.
	 * \return Results.
	 */
	virtual FetchResult parseResponse(QNetworkReply* response,
			const FetchRequest& originalRequest) const = 0;

	/*!
	 * \brief Gets all DataTypes which are implemented in this provider.
	 * \return Implemented DataTypes.
	 */
	virtual std::vector<DataType> getImplementedDataTypes() const = 0;

	/*!
	 * \brief Check if dataType is implemented (and can be fetched).
	 * \param dataType
	 * \return True if implemented, false otherwise.
	 */
	virtual bool implementsDataType(DataType dataType) const = 0;

	/*!
	 * \brief Tries to parse error from QNetworkReply. If error is unknown to
	 * us, just returns FetchError::NONE.
	 * \param response
	 * \return FetchError based on reply.
	 */
	virtual FetchError parseError(QNetworkReply* response) const = 0;
};

}  // namespace pwa
#endif  // IPROVIDER_H
